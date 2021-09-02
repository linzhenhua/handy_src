#ifndef COMMON_URI_PARSER_H_
#define COMMON_URI_PARSER_H_

#include <vector>
#include <string>

#include <boost/noncopyable.hpp>

namespace common {

class UriParser final : boost::noncopyable {
public:
    explicit UriParser(const std::string& uri) noexcept;
    ~UriParser() noexcept;

    const std::string& GetScheme() const;
    const std::string& GetUserInfo() const;
    const std::string& GetHost() const;
    unsigned GetPort() const;
    const std::string& GetPath() const;
    const std::string& GetQuery() const;
    std::vector<std::pair<std::string, std::string>> GetQueryParameters() const;
    const std::string& GetFragment() const;

private:
    void Parse();
    void ParseAuthority(std::string::const_iterator& it, std::string::const_iterator& end);
    void ParseHostAndPort(std::string::const_iterator& it, const std::string::const_iterator& end);
    void ParseFullPath(std::string::const_iterator& it, const std::string::const_iterator& end);
    void ParsePath(std::string::const_iterator& it, const std::string::const_iterator& end);
    void ParseQuery(std::string::const_iterator& it, const std::string::const_iterator& end);
    void ParseFragment(std::string::const_iterator& it, const std::string::const_iterator& end);

    static void Decode(const std::string& str, std::string& decoded);
    unsigned GetDefaultPort() const;

    std::string uri_;
    std::string scheme_;
    std::string user_info_;
    std::string host_;
    unsigned port_{0};
    std::string path_;
    std::string query_;
    std::string fragment_;

};

}

#endif //COMMON_URI_PARSER_H_
