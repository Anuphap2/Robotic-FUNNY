#ifndef WEB_API_H
#define WEB_API_H
#include <WebServer.h>
#include <sys/socket.h>
#include "RuntimeConfig.h"

// ESP32 core 3.3.12 raw-body callbacks let us bound POST storage before the
// stock form parser allocates argument Strings. No JSON parser is needed.
class RobotWebServer : public WebServer {
public:
  explicit RobotWebServer(int port) : WebServer(port) {}
  // The dashboard has no streaming/SSE handlers or middleware. Avoid the stock
  // five-second idle connection wait, and use short stream idle timeouts.
  void handleClient() override {
    if (_currentStatus == HC_NONE) {
      _currentClient = _server.accept();
      if (!_currentClient) return;
      _currentClient.setNoDelay(true);
      _currentClient.setTimeout(100);
      _currentStatus = HC_WAIT_READ;
      _statusChange = millis();
    }
    if (_currentClient.connected() && !_currentClient.available() && millis() - _statusChange < 150) return;
    if (_currentClient.connected() && _currentClient.available()) {
      _writing = false;
      if (_parseRequest(_currentClient)) {
        _writing = false;
        _contentLength = CONTENT_LENGTH_NOT_SET;
        _responseCode = 0;
        _clearResponseHeaders();
        _handleRequest();
      }
    }
    _currentClient.stop();
    _currentClient = NetworkClient();
    _currentStatus = HC_NONE;
    _currentUpload.reset(); _currentRaw.reset();
  }
  void receiveBody() {
    if (hasUpload()) { rejectBody(415, "form_encoding_required"); return; }
    HTTPRaw& body = raw();
    if (body.status == RAW_START) {
      _bodySize = 0; _bodyReady = false;
      if (header("X-Robot-Request") != "1") { rejectBody(403, "robot_header_required"); return; }
      if (!header("Content-Type").startsWith("application/x-www-form-urlencoded")) {
        rejectBody(415, "form_encoding_required"); return;
      }
      if (clientContentLength() < 0 || clientContentLength() > (int)BODY_LIMIT) {
        rejectBody(413, "config_request_too_large"); return;
      }
    } else if (body.status == RAW_WRITE) {
      if (_bodySize + body.currentSize > BODY_LIMIT || memchr(body.buf, 0, body.currentSize)) {
        rejectBody(413, "invalid_request_body"); return;
      }
      memcpy(_body + _bodySize, body.buf, body.currentSize);
      _bodySize += body.currentSize;
    } else if (body.status == RAW_END) {
      _body[_bodySize] = 0;
      size_t pairs = _bodySize ? 1 : 0;
      for (size_t i = 0; i < _bodySize; ++i) if (_body[i] == '&') ++pairs;
      if (pairs > CONFIG_FIELD_COUNT + 1) { rejectBody(413, "too_many_arguments"); return; }
      _parseArguments(String(_body));
      _bodyReady = true;
    } else if (body.status == RAW_ABORTED) _bodyReady = false;
  }
  bool bodyReady() const { return _bodyReady; }
protected:
  // NetworkClient's stock write can retry for seconds. Bound the complete
  // response instead, including chunked schema/catalog and the flash HTML.
  size_t _currentClientWrite(const char* data, size_t length) override {
    if (!_writing) { _writeStarted = millis(); _writing = true; }
    size_t sent = 0;
    while (sent < length && _currentClient.connected() && millis() - _writeStarted < 350) {
      int count = ::send(_currentClient.fd(), data + sent, length - sent, MSG_DONTWAIT);
      if (count > 0) sent += count;
      else if (count < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) yield();
      else break;
    }
    if (sent != length) _currentClient.stop();
    return sent;
  }
  size_t _currentClientWrite_P(PGM_P data, size_t length) override {
    return _currentClientWrite(data, length);
  }
private:
  uint32_t _writeStarted = 0;
  bool _writing = false;
  static constexpr size_t BODY_LIMIT = 2048;
  char _body[BODY_LIMIT + 1] = {};
  size_t _bodySize = 0;
  bool _bodyReady = false;
  void rejectBody(int code, const char* error) {
    _bodyReady = false;
    char json[120]; snprintf(json, sizeof(json), "{\"ok\":false,\"error\":\"%s\"}", error);
    send(code, "application/json", json);
    client().stop();
  }
};

// Small, fixed buffer for streamed JSON. Catalog/schema size never becomes a
// single large String on the ESP32 heap. Strings are escaped, including controls.
class JsonResponse {
public:
  explicit JsonResponse(WebServer& server) : _server(server) {
    _server.sendHeader("Cache-Control", "no-store");
    _server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    _server.send(200, "application/json", "");
  }
  void raw(const char* value) { while (*value) put(*value++); }
  void number(int64_t value) {
    char buffer[24]; snprintf(buffer, sizeof(buffer), "%lld", (long long)value); raw(buffer);
  }
  void boolean(bool value) { raw(value ? "true" : "false"); }
  void quoted(const char* value) {
    put('"');
    while (*value) {
      uint8_t c = static_cast<uint8_t>(*value++);
      if (c == '"' || c == '\\') { put('\\'); put(c); }
      else if (c < 32) {
        char escape[7]; snprintf(escape, sizeof(escape), "\\u%04x", c); raw(escape);
      } else put(c);
    }
    put('"');
  }
  void key(const char* value) { quoted(value); put(':'); }
  void finish() { flush(); _server.sendContent(""); }
private:
  WebServer& _server;
  char _buffer[384];
  size_t _used = 0;
  void put(char c) { if (_used == sizeof(_buffer)) flush(); _buffer[_used++] = c; }
  void flush() {
    if (_used) { _server.sendContent(_buffer, _used); _used = 0; }
  }
};
#endif
