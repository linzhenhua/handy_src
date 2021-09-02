#include "uri_parser.h"

#include <boost/algorithm/string.hpp>

namespace common {

UriParser::UriParser(const std::string& uri) noexcept : uri_{uri} {
    Parse();
}

UriParser::~UriParser() noexcept = default;

const std::string& UriParser::GetScheme() const {
    return scheme_;
}

const std::string& UriParser::GetUserInfo() const {
    return user_info_;
}

const std::string& UriParser::GetHost() const {
    return host_;
}

unsigned UriParser::GetPort() const {
    return port_;
}

const std::string& UriParser::GetPath() const {
    return path_;
}

const std::string& UriParser::GetQuery() const {
    return query_;
}

std::vector<std::pair<std::string, std::string>> UriParser::GetQueryParameters() const {
    std::vector<std::pair<std::string, std::string>> result;
    std::string::const_iterator it{query_.begin()};
    std::string::const_iterator end{query_.end()};
    while (it != end) {
        std::string name;
        std::string value;
        while (it != end && *it != '=' && *it != '&') {
            if (*it == '+') {
                name += ' ';
            } else {
                name += *it;
            }
            ++it;
        }
        if (it != end && *it == '=') {
            ++it;
            while (it != end && *it != '&') {
                if (*it == '+') {
                    value += ' ';
                } else {
                    value += *it;
                }
                ++it;
            }
        }
        std::string decoded_name;
        std::string decoded_value;
        Decode(name, decoded_name);
        Decode(value, decoded_value);
        result.emplace_back(std::make_pair(decoded_name, decoded_value));
        if (it != end && *it == '&') {
            ++it;
        }
    }
    return result;
}

const std::string& UriParser::GetFragment() const {
    return fragment_;
}

void UriParser::Parse() {
    if (uri_.empty()) {
        return;
    }

    auto it = uri_.cbegin();
    auto end = uri_.cend();

    if (*it != '/' && *it != '.' && *it != '?' && *it != '#') {
        std::string scheme;
        while (it != end && *it != ':' && *it != '?' && *it != '#' && *it != '/') {
            scheme += *it++;
        }

        if (it != end && *it == ':') {
            ++it;
            if (it == end) {
                return;
            }

            scheme_ = scheme;
            boost::to_lower(scheme_);

            if (*it == '/') {
                ++it;
                if (it != end && *it == '/') {
                    ++it;
                    ParseAuthority(it, end);
                } else {
                    --it;
                }
            }
            ParseFullPath(it, end);
        } else {
            it = uri_.begin();
            ParseFullPath(it, end);
        }
    } else {
        ParseFullPath(it, end);
    }
}

void UriParser::ParseAuthority(std::string::const_iterator& it, std::string::const_iterator& end) {
    std::string user_info;
    std::string part;
    while (it != end && *it != '/' && *it != '?' && *it != '#') {
        if (*it == '@') {
            user_info = part;
            part.clear();
        } else {
            part += *it;
        }
        ++it;
    }
    auto part_begin = part.cbegin();
    const auto part_end = part.cend();
    ParseHostAndPort(part_begin, part_end);
    user_info_ = user_info;
}

void UriParser::ParseHostAndPort(std::string::const_iterator& it, const std::string::const_iterator& end) {
    if (it == end) {
        return;
    }

    std::string host;
    if (*it == '[') {
        // IPv6 address
        ++it;
        while (it != end && *it != ']') {
            host += *it++;
        }
        if (it == end) {
            return;
        }
        ++it;
    } else {
        while (it != end && *it != ':') {
            host += *it++;
        }
    }
    if (it != end && *it == ':') {
        ++it;
        std::string port;
        while (it != end) {
            port += *it++;
        }

        if (!port.empty()) {
            port_ = std::stoi(port);
        } else {
            port_ = GetDefaultPort();
        }
    } else {
        port_ = GetDefaultPort();
    }
    host_ = host;
    boost::to_lower(host_);
}

void UriParser::ParseFullPath(std::string::const_iterator& it, const std::string::const_iterator& end) {
    if (it == end) {
        return;
    }

    if (*it != '?' && *it != '#') {
        ParsePath(it, end);
    }

    if (it != end && *it == '?') {
        ++it;
        ParseQuery(it, end);
    }

    if (it != end && *it == '#') {
        ++it;
        ParseFragment(it, end);
    }
}

void UriParser::ParsePath(std::string::const_iterator& it, const std::string::const_iterator& end) {
    std::string path;
    while (it != end && *it != '?' && *it != '#') {
        path += *it++;
    }

    Decode(path, path_);
}

void UriParser::ParseQuery(std::string::const_iterator& it, const std::string::const_iterator& end) {
    query_.clear();
    while (it != end && *it != '#') {
        query_ += *it++;
    }
}

void UriParser::ParseFragment(std::string::const_iterator& it, const std::string::const_iterator& end) {
    std::string fragment;
    while (it != end) {
        fragment += *it++;
    }
    Decode(fragment, fragment_);
}

void UriParser::Decode(const std::string& str, std::string& decoded) {
    auto it = str.cbegin();
    auto end = str.cend();
    while (it != end) {
        char c{*it++};
        if (c == '%') {
            if (it == end) {
                return;
            }
            char hi{*it++};
            if (it == end) {
                return;
            }
            char lo{*it++};
            if (hi >= '0' && hi <= '9') {
                c = hi - '0';
            } else if (hi >= 'A' && hi <= 'F') {
                c = hi - 'A' + 10;
            } else if (hi >= 'a' && hi <= 'f') {
                c = hi - 'a' + 10;
            } else {
                return;
            }
            c *= 16;
            if (lo >= '0' && lo <= '9') {
                c += lo - '0';
            } else if (lo >= 'A' && lo <= 'F') {
                c += lo - 'A' + 10;
            } else if (lo >= 'a' && lo <= 'f') {
                c += lo - 'a' + 10;
            } else {
                return;
            }
        }
        decoded += c;
    }
}

unsigned UriParser::GetDefaultPort() const {
    if (scheme_ == "rtmp") {
        return 1935;
    } else if (scheme_ == "http" || scheme_ == "ws") {
        return 80;
    } else if (scheme_ == "https" || scheme_ == "wss") {
        return 443;
    } else if (scheme_ == "rtsp") {
        return 554;
    }

    return 0;
}

}
