//Credits: https://github.com/aixee/cstrike-basehook-linux

#pragma once
#ifndef SDK_HPP
#define SDK_HPP

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>

inline void**& GetVirtualTable(void* baseclass)
{
	return *reinterpret_cast<void***>(baseclass);
}

template <typename Fn = void*> inline Fn GetVirtualFunction(void* baseclass, size_t index)
{
	return reinterpret_cast<Fn>(GetVirtualTable(baseclass)[index]);
}


class VMTHook {
	private:
		std::uintptr_t** baseclass = nullptr;
		std::unique_ptr<std::uintptr_t[]> current_vft = nullptr;
		std::uintptr_t* original_vft = nullptr;
		std::size_t total_functions = 0;
	public:
		VMTHook(void) = default;

		VMTHook(void* baseclass) {
			this->baseclass = static_cast<std::uintptr_t**>(baseclass);

			while (static_cast<std::uintptr_t*>(*this->baseclass)[this->total_functions])
				++this->total_functions;

			const std::size_t table_size = this->total_functions * sizeof(std::uintptr_t);

			this->original_vft = *this->baseclass;
			this->current_vft = std::make_unique<std::uintptr_t[]>(this->total_functions);

			std::memcpy(this->current_vft.get(), this->original_vft, table_size);

			*this->baseclass = this->current_vft.get();
		};

		~VMTHook() {
			*this->baseclass = this->original_vft;
		};

		inline void* GetOriginalFunction(std::size_t function_index) {
			return reinterpret_cast<void*>(this->original_vft[function_index]);
		}

		template <typename Function> inline const Function GetOriginalFunction(std::size_t function_index) {
			return reinterpret_cast<Function>(this->original_vft[function_index]);
		}

		inline bool HookFunction(void* new_function, const std::size_t function_index) {
			if (function_index > this->total_functions)
				return false;

			this->current_vft[function_index] = reinterpret_cast<std::uintptr_t>(new_function);

			return true;
		}

		inline bool UnhookFunction(const std::size_t function_index) {
			if (function_index > this->total_functions)
				return false;

			this->current_vft[function_index] = this->original_vft[function_index];

			return true;
		}

		inline std::size_t GetTotalFunctions() {
			return this->total_functions;
		}
};


class CShowPixelsParams
{
	public:
		unsigned int m_srcTexName;
		int m_width;
		int m_height;
		bool m_vsyncEnable;
		bool m_fsEnable;
		bool m_useBlit;
		bool m_noBlit;
		bool m_onlySyncView;
};

typedef void* PseudoGLContextPtr;

class ILauncherMgr {
	public:
		void PumpWindowsMessageLoop()
        {
			return GetVirtualFunction<void(*)(ILauncherMgr*)>(this, 15)(this);
		}

		PseudoGLContextPtr GetMainContext()
        {
			return GetVirtualFunction<PseudoGLContextPtr(*)(ILauncherMgr*)>(this, 22)(this);
		}

		PseudoGLContextPtr CreateExtraContext()
        {
			return GetVirtualFunction<PseudoGLContextPtr(*)(ILauncherMgr*)>(this, 24)(this);
		}

		void DeleteContext(PseudoGLContextPtr context)
        {
			return GetVirtualFunction<void(*)(ILauncherMgr*, PseudoGLContextPtr)>(this, 27)(this, context);
		}

		bool MakeContextCurrent(PseudoGLContextPtr context)
        {
			return GetVirtualFunction<bool(*)(ILauncherMgr*, PseudoGLContextPtr)>(this, 26)(this, context);
		}

		void PumpWindowsMessageLoop(CShowPixelsParams* params)
        {
			return GetVirtualFunction<void(*)(ILauncherMgr*, CShowPixelsParams*)>(this, 29)(this, params);
		}

		void* GetWindowRef()
        {
			return GetVirtualFunction<void*(*)(ILauncherMgr*)>(this, 32)(this);
		}
};

#endif