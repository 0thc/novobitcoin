// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include <rpc/client_config.h>

namespace rpc::client
{

class HTTPRequest;
class HTTPResponse;

/**
 * Class to perform HTTP RPC/REST requests.
 */
class RPCClient
{
  public:
    explicit RPCClient(const RPCClientConfig& config) : mConfig{config} {}

    // Submit a request and wait for a response
    void SubmitRequest(HTTPRequest& request, HTTPResponse* response) const;

  private:

    // Config to describe the required connection type
    RPCClientConfig mConfig {};
};

}
