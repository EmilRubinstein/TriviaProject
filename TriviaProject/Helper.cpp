#include "Helper.h"
#include <iostream>
#include <cstring>
#include <WinSock2.h>

void printRequest(const RequestInfo& req)
{
    std::cout << "\n[Server] Request received:\n";
    std::cout << "Code: " << (int)req.code << "\n";
    std::cout << "Size: " << req.json_raw_data.size() << "\n";
    std::cout << "Raw JSON: ";

    try
    {
        // Convert raw byte buffer into string for debug printing
        std::string s(req.json_raw_data.begin(), req.json_raw_data.end());
        std::cout << s << "\n";
    }
    catch (...)
    {
        // Handles invalid or non-printable buffer content
        std::cout << "[invalid data]\n";
    }
}

void printResponse(const RequestResult& res)
{
    std::cout << "\n[Server] Sending response:\n";
    std::cout << "Size: " << res.response.size() << "\n";

    if (res.response.empty())
    {
        std::cout << "Empty response\n\n";
        return;
    }

    try
    {
        // First byte is response code
        int code = (int)res.response[0];
        std::cout << "Code: " << code << "\n";

        // Next 4 bytes represent JSON size (little-endian)
        if (res.response.size() > 5)
        {
            int size = 0;

            // Copy 4 bytes safely into integer
            std::memcpy(&size, res.response.data() + 1, 4);

            // Extract JSON payload starting after header (1 byte code + 4 bytes size)
            std::string jsonStr(
                res.response.begin() + 5,
                res.response.begin() + 5 + size
            );

            std::cout << "JSON: " << jsonStr << "\n";
        }
    }
    catch (...)
    {
        // Handles malformed packet or corrupted memory access
        std::cout << "[invalid response format]\n";
    }
}

int readFully(SOCKET s, char* buffer, int len)
{
    int total = 0;

    // Ensures full TCP message is received (handles partial recv)
    while (total < len)
    {
        int r = recv(s, buffer + total, len - total, 0);

        if (r <= 0)
            return r;

        total += r;
    }

    return total;
}