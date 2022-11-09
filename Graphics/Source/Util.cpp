#include "stdafx.h"

#include "Util.h"

namespace
{
    class TimeHelper
    {
    public:
        TimeHelper()
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);
            m_frequency = frequency.QuadPart;

            LARGE_INTEGER offset;
            QueryPerformanceCounter(&offset);
            m_offset = offset.QuadPart;
        }

        void Reset()
        {
            LARGE_INTEGER offset;
            QueryPerformanceCounter(&offset);
            m_offset = offset.QuadPart;
        }

        double Get()
        {
            LARGE_INTEGER cnt;
            QueryPerformanceCounter(&cnt);
            return 1.0 * (cnt.QuadPart - m_offset) / m_frequency;
        }

    private:
        int64_t m_frequency;
        int64_t m_offset;
    };

    // Global object
    TimeHelper timeHelper;
}

namespace dx
{
    std::string WstringToString(const std::wstring& wstr)
    {
        auto size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()),
            nullptr, 0, nullptr, nullptr);
        std::string ret(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, ret.data(), size, nullptr, nullptr);
        return ret;
    }

    std::wstring StringToWstring(const std::string& str)
    {
        auto size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
        std::wstring ret(size, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, ret.data(), size);
        return ret;
    }

    // Get time in seconds since either the app started or ResetTime() was called
    double GetTime()
    {
        return timeHelper.Get();
    }

    void ResetTime()
    {
        timeHelper.Reset();
    }

    void SetVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* pShader)
    {
        pContext->VSSetShader(pShader, nullptr, 0);
    }

    void SetPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* pShader)
    {
        pContext->PSSetShader(pShader, nullptr, 0);
    }

    void SetCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pShader)
    {
        pContext->CSSetShader(pShader, nullptr, 0);
    }
}