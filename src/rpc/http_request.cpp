// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <rpc/http_request.h>
#include <rpc/protocol.h>

namespace rpc::client
{

// Create a properly formatted JSONRPCRequest
HTTPRequest HTTPRequest::CreateJSONRPCRequest(const RPCClientConfig& config, const std::string& method, const UniValue& params)
{
    // Format contents
    std::string contents { JSONRPCRequestObj(method, params, 1).write() + "\n" };

    // Format endpoint
    std::string endPoint {"/"};
    if(!config.GetWallet().empty())
    {
        // We need to pass a wallet ID via the URI
        endPoint = "/wallet/" + EncodeURI(config.GetWallet());
    }

    // Create request
    return { endPoint, contents, RequestCmdType::POST };
}

// Create a properly formatted query request to a double-spend endpoint
HTTPRequest HTTPRequest::CreateDSEndpointQueryRequest(const RPCClientConfig& config, const std::string& txid)
{
    // Format endpoint
    std::string endpoint { config.GetEndpoint() + "query/" + txid };

    // Create request
    return { endpoint, RequestCmdType::GET };
}

}
