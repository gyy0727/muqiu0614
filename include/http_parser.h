#ifndef HTTP_PARSER_H__
#define HTTP_PARSER_H__

#include "http.h"
#include "http11_common.h"
#include "http11_parser.h"
#include "httpclient_parser.h"
//* HTTP请求解析类
class HttpRequestParser {
public:
  /// HTTP解析类的智能指针
  typedef std::shared_ptr<HttpRequestParser> ptr;

  HttpRequestParser();

  //*解析数据,返回已解析的长度
  size_t execute(char *data, size_t len);

  //*是否解析完成
  int isFinished();

  //*是否有错误
  int hasError();

  //*返回request结构体
  HttpRequest::ptr getData() const { return m_data; }

  //*设置错误
  void setError(int v) { m_error = v; }

  //*获取消息体长度
  uint64_t getContentLength();

  const http_parser &getParser() const { return m_parser; }

public:
  //*   返回HttpRequest协议解析的缓存大小
  static uint64_t GetHttpRequestBufferSize();

  //   * @brief 返回HttpRequest协议的最大消息体大小

  static uint64_t GetHttpRequestMaxBodySize();

private:
  /// http_parser
  http_parser m_parser;
  /// HttpRequest结构
  HttpRequest::ptr m_data;
  /// 错误码
  /// 1000: invalid method
  /// 1001: invalid version
  /// 1002: invalid field
  int m_error;
};

/**
 * @brief Http响应解析结构体
 */
class HttpResponseParser {
public:
  /// 智能指针类型
  typedef std::shared_ptr<HttpResponseParser> ptr;

  HttpResponseParser();

  //*解析响应数据,返回已解析的长度
  size_t execute(char *data, size_t len, bool chunck);

  //* 是否解析完成
  int isFinished();

  //*是否有错误
  int hasError();

  //* 返回HttpResponse
  HttpResponse::ptr getData() const { return m_data; }

  //*设置错误码
  void setError(int v) { m_error = v; }

  //*获取消息体长度
  uint64_t getContentLength();

  //* 返回httpclient_parser
  const httpclient_parser &getParser() const { return m_parser; }

public:
  //*返回HTTP响应解析缓存大小
  static uint64_t GetHttpResponseBufferSize();

  //*最大消息体的大小
  static uint64_t GetHttpResponseMaxBodySize();

private:
  /// httpclient_parser
  httpclient_parser m_parser;
  /// HttpResponse
  HttpResponse::ptr m_data;
  /// 错误码
  /// 1001: invalid version
  /// 1002: invalid field
  int m_error;
};

#endif
