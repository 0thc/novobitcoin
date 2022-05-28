// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include <streams.h>
#include <version.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace rpc::client
{

/**
 * Base class for HTTP RPC/REST responses.
 */
class HTTPResponse
{
  public:
    HTTPResponse() = default;
    HTTPResponse(const std::vector<std::string>& expectedHeaders) : mExpectedHeaders{expectedHeaders} {}
    virtual ~HTTPResponse() = default;

    void SetStatus(int status) { mStatus = status; };
    void SetError(int error) { mError = error; }
    int GetStatus() const { return mStatus; }
    int GetError() const { return mError; }

    const std::vector<std::string>& GetExpectedHeaders() const { return mExpectedHeaders; }
    const std::unordered_map<std::string, std::string>& GetHeaders() const { return mHeaders; }
    void SetHeaderValue(const std::string& header, const std::string& value) { mHeaders[header] = value; }

    virtual void SetBody(const unsigned char* body, size_t size) = 0;
    virtual bool IsEmpty() const = 0;

  private:

    int mStatus {0};
    int mError  {0};

    std::vector<std::string> mExpectedHeaders {};
    std::unordered_map<std::string, std::string> mHeaders {};
};

/**
 * A string formatted HTTP response
 */
class StringHTTPResponse : public HTTPResponse
{
  public:
    StringHTTPResponse() = default;
    StringHTTPResponse(const std::vector<std::string>& expectedHeaders) : HTTPResponse{expectedHeaders} {}

    const std::string& GetBody() const { return mBody; }

    void SetBody(const unsigned char* body, size_t size) override;
    bool IsEmpty() const override { return mBody.empty(); }

  private:
    std::string mBody {};
};

/**
 * A binary (byte array) HTTP response
 */
class BinaryHTTPResponse : public HTTPResponse
{
  public:
    BinaryHTTPResponse() = default;
    BinaryHTTPResponse(const std::vector<std::string>& expectedHeaders) : HTTPResponse{expectedHeaders} {}

    const std::vector<uint8_t>& GetBody() const { return mBody; }

    void SetBody(const unsigned char* body, size_t size) override;
    bool IsEmpty() const override { return mBody.empty(); }

    // Helper for deserialising binary responses
    template<typename T>
    BinaryHTTPResponse& operator>>(T&& obj)
    {
        CDataStream stream { mBody, SER_NETWORK, PROTOCOL_VERSION };
        stream >> std::forward<T>(obj);
        return *this;
    }

  private:
    std::vector<uint8_t> mBody {};
};

}
