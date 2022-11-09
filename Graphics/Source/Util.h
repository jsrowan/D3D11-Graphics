#pragma once

namespace dx
{
    template<typename... Args>
    class Signal
    {
    public:
        using Index = size_t;

        Signal() = default;

        void Send(Args... args) const
        {
            for (auto& slot : m_connected)
            {
                slot(args...);
            }
        }

        Index Register(const std::function<void(Args...)>& slot)
        {
            m_connected.push_back(slot);
            return m_connected.size() - 1;
        }

        template<typename T>
        Index Register(T* instance, void (T::* func)(Args...))
        {
            return Register([=](Args... args) { (instance->*func)(args...); });
        }

        template<typename T>
        Index Register(const T* instance, void (T::* func)(Args...) const)
        {
            return Register([=](Args... args) { (instance->*func)(args...); });
        }

        void Deregister(Index id)
        {
            Index index = 0;
            for (const auto& it = m_connected.cbegin(); it != m_connected.cend(); ++it)
            {
                if (index == id)
                {
                    m_connected.erase(it);
                    return;
                }
                index++;
            }
        }

    private:
        std::vector<std::function<void(Args...)>> m_connected;
    };

    template<typename T>
    constexpr auto EnumToInt(T e)
    {
        static_assert(std::is_enum_v<T>, "EnumToInt only works for enums");
        return static_cast<std::underlying_type_t<T>>(e);
    }

    template<typename T>
    constexpr T IntToEnum(std::underlying_type_t<T> i)
    {
        static_assert(std::is_enum_v<T>, "IntToEnum only works for enums");
        return static_cast<T>(i);
    }

    template<typename T>
    struct BitmaskEnable
    {
        static constexpr bool enabled = false;
    };

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T operator|(T lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return static_cast<T>(static_cast<Type>(lhs) | static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T operator&(T lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return static_cast<T>(static_cast<Type>(lhs) & static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T operator^(T lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return static_cast<T>(static_cast<Type>(lhs) ^ static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T operator~(T a)
    {
        using Type = std::underlying_type_t<T>;

        return static_cast<T>(~static_cast<Type>(a));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T& operator|=(T& lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return reinterpret_cast<T&>(reinterpret_cast<Type&>(lhs) |= static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T& operator&=(T& lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return reinterpret_cast<T&>(reinterpret_cast<Type&>(lhs) &= static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr T& operator^=(T& lhs, T rhs)
    {
        using Type = std::underlying_type_t<T>;

        return reinterpret_cast<T&>(reinterpret_cast<Type&>(lhs) ^= static_cast<Type>(rhs));
    }

    template<typename T, typename = std::enable_if_t<BitmaskEnable<T>::enabled>>
    constexpr bool Any(T a)
    {
        using Type = std::underlying_type_t<T>;

        return static_cast<Type>(a) != 0;
    }

    // Converts a POD struct into a string
    template<typename T>
    std::string CreateKey(const T& s)
    {
        std::array<unsigned char, sizeof(T)> arr{};
        memcpy(arr.data(), &s, sizeof(T));
        std::stringstream ss;
        for (const auto& val : arr)
        {
            ss << val;
        }
        return ss.str();
    }

    // String conversion functions, assuming std::string uses UTF-8
    std::string WstringToString(const std::wstring& wstr);
    std::wstring StringToWstring(const std::string& str);

    // Returns time from app start in seconds
    double GetTime();
    void ResetTime();

    // D3D11 Helper Functions for setting and clearing device state without creating temporary arrays
    template<typename... Args>
    void BindShaderResourcesVS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11ShaderResourceView*, 16> nullSRVs = { nullptr };
            pContext->VSSetShaderResources(0, static_cast<unsigned int>(nullSRVs.size()), nullSRVs.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->VSSetShaderResources(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11ShaderResourceView*, sizeof...(Args)> srvs = { args... };
            pContext->VSSetShaderResources(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindShaderResourcesPS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {
        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11ShaderResourceView*, 16> nullSRVs = { nullptr };
            pContext->PSSetShaderResources(0, static_cast<unsigned int>(nullSRVs.size()), nullSRVs.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->PSSetShaderResources(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11ShaderResourceView*, sizeof...(Args)> srvs = { args... };
            pContext->PSSetShaderResources(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindShaderResourcesCS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11ShaderResourceView*, 16> nullSRVs = { nullptr };
            pContext->CSSetShaderResources(0, static_cast<unsigned int>(nullSRVs.size()), nullSRVs.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->CSSetShaderResources(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11ShaderResourceView*, sizeof...(Args)> srvs = { args... };
            pContext->CSSetShaderResources(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindUnorderedAccessViewsCS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11UnorderedAccessView*, 16> nullUAVs = { nullptr };
            pContext->CSSetUnorderedAccessViews(0, static_cast<unsigned int>(nullUAVs.size()), nullUAVs.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->CSSetUnorderedAccessViews(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11UnorderedAccessView*, sizeof...(Args)> srvs = { args... };
            pContext->CSSetUnorderedAccessViews(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindSamplersVS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11SamplerState*, 16> nullSamplers = { nullptr };
            pContext->VSSetSamplers(0, static_cast<unsigned int>(nullSamplers.size()), nullSamplers.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->VSSetSamplers(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11SamplerState*, sizeof...(Args)> srvs = { args... };
            pContext->VSSetSamplers(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindSamplersPS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11SamplerState*, 16> nullSamplers = { nullptr };
            pContext->PSSetSamplers(0, static_cast<unsigned int>(nullSamplers.size()), nullSamplers.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->PSSetSamplers(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11SamplerState*, sizeof...(Args)> srvs = { args... };
            pContext->PSSetSamplers(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    template<typename... Args>
    void BindSamplersCS(ID3D11DeviceContext* pContext, unsigned int slot, Args... args)
    {

        if constexpr (sizeof...(Args) == 0)
        {
            std::array<ID3D11SamplerState*, 16> nullSamplers = { nullptr };
            pContext->CSSetSamplers(0, static_cast<unsigned int>(nullSamplers.size()), nullSamplers.data());
        }
        else if constexpr (sizeof...(Args) == 1)
        {
            pContext->CSSetSamplers(slot, 1, &args...);
        }
        else
        {
            std::array<ID3D11SamplerState*, sizeof...(Args)> srvs = { args... };
            pContext->CSSetSamplers(slot, static_cast<unsigned int>(srvs.size()), srvs.data());
        }
    }

    void SetVS(ID3D11DeviceContext* pContext, ID3D11VertexShader* pShader);
    void SetPS(ID3D11DeviceContext* pContext, ID3D11PixelShader* pShader);
    void SetCS(ID3D11DeviceContext* pContext, ID3D11ComputeShader* pShader);

    // Sets or clears all 8 render targets. pDSV can be null if there is no depth target.
    template<typename... Args>
    void BindRenderTargets(ID3D11DeviceContext* pContext, ID3D11DepthStencilView* pDSV, const Args&... args)
    {
        std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> rtvs = { args... };
        pContext->OMSetRenderTargets(static_cast<unsigned int>(rtvs.size()), rtvs.data(), pDSV);
    }
}

