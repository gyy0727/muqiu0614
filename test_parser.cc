#include <cstring>
#include <iostream>
#include <string>
// 回调函数类型
typedef void (*element_cb)(const char* at, size_t length);

// 示例回调函数
void on_request_method(const char* at, size_t length) {
    std::string method(at, length);
    std::cout << "Request Method: " << method << std::endl;
}

void on_request_uri(const char* at, size_t length) {
    std::string uri(at, length);
    std::cout << "Request URI: " << uri << std::endl;
}

void on_fragment(const char* at, size_t length) {
    std::string fragment(at, length);
    std::cout << "Fragment: " << fragment << std::endl;
}

void on_request_path(const char* at, size_t length) {
    std::string path(at, length);
    std::cout << "Request Path: " << path << std::endl;
}

void on_query_string(const char* at, size_t length) {
    std::string query(at, length);
    std::cout << "Query String: " << query << std::endl;
}

void on_http_version(const char* at, size_t length) {
    std::string version(at, length);
    std::cout << "HTTP Version: " << version << std::endl;
}

void on_header_done(const char* at, size_t length) {
    std::cout << "Headers done!" << std::endl;
}
// HTTP 请求解析器
typedef struct http_parser {
    int cs;
    size_t body_start;
    int content_len;
    size_t nread;
    size_t mark;
    size_t field_start;
    size_t field_len;
    size_t query_start;
    int xml_sent;
    int json_sent;
    void *data;
    int uri_relaxed;
    element_cb request_method;
    element_cb request_uri;
    element_cb fragment;
    element_cb request_path;
    element_cb query_string;
    element_cb http_version;
    element_cb header_done;
} http_parser;

// 简单的解析函数
void parse_http_request(http_parser* parser, const char* request) {
    // 解析请求行
    const char* method_end = strchr(request, ' ');
    if (method_end) {
        parser->request_method(request, method_end - request);
        const char* uri_start = method_end + 1;
        const char* uri_end = strchr(uri_start, ' ');
        if (uri_end) {
            parser->request_uri(uri_start, uri_end - uri_start);
            const char* version_start = uri_end + 1;
            const char* version_end = strchr(version_start, '\r');
            if (version_end) {
                parser->http_version(version_start, version_end - version_start);
            }
        }
    }

    // 假设查询字符串和片段已经包含在 request_uri 中，需要进一步解析
    const char* query_start = strchr(request, '?');
    if (query_start) {
        const char* fragment_start = strchr(query_start, '#');
        if (fragment_start) {
            parser->query_string(query_start + 1, fragment_start - query_start - 1);
            parser->fragment(fragment_start + 1, strlen(fragment_start + 1));
        } else {
            parser->query_string(query_start + 1, strlen(query_start + 1));
        }
    }

    // 请求路径（去掉查询字符串和片段）
    const char* path_end = query_start ? query_start : strchr(request, ' ');
    if (path_end) {
        parser->request_path(request, path_end - request);
    }

    // 模拟处理完所有头部
    parser->header_done(nullptr, 0);
}

int main() {
    // 创建并初始化解析器
    http_parser parser;
    parser.request_method = on_request_method;
    parser.request_uri = on_request_uri;
    parser.fragment = on_fragment;
    parser.request_path = on_request_path;
    parser.query_string = on_query_string;
    parser.http_version = on_http_version;
    parser.header_done = on_header_done;

    // 示例 HTTP 请求
    const char* http_request = 
        "GET /index.html?name=value#section1 HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "User-Agent: curl/7.68.0\r\n"
        "Accept: */*\r\n"
        "\r\n";

    // 解析 HTTP 请求
    parse_http_request(&parser, http_request);

    return 0;
}

