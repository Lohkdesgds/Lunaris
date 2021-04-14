#pragma once

// C
#include <windows.h>
#include <wininet.h>
#include <ShlObj.h>
// C++
#include <string>
#include <thread>
#include <functional>

#pragma comment (lib, "wininet.lib")

#include "../../Tools/Buffer/buffer.h"
#include "../../Tools/Future/future.h"
#include "../../Tools/SuperThread/superthread.h"


namespace LSW {
	namespace v5 {
		namespace Interface {

			namespace download {
				constexpr auto max_block_size = 8192;
			}

			class Downloader {
				std::shared_ptr<Tools::Buffer> buf = std::make_shared<Tools::Buffer>();
				Tools::SuperThread<std::shared_ptr<Tools::Buffer>> thr{ Tools::superthread::performance_mode::HIGH_PERFORMANCE };
				size_t TotalBytesRead = 0;

				bool _get(const std::string&, std::function<bool(void)>);
			public:
				~Downloader();

				/// <summary>
				/// <para>Download content from this URL and stores in a internal Buffer.</para>
				/// </summary>
				/// <param name="{std::string}">The URL.</param>
				/// <returns>{bool} True if succeess.</returns>
				bool get(const std::string&);

				/// <summary>
				/// <para>Download content from this URL asynchronously.</para>
				/// </summary>
				/// <param name="{std::string}">The URL.</param>
				/// <returns>{Future} The data downloaded.</returns>
				Tools::Future<std::shared_ptr<Tools::Buffer>> get_async(const std::string&);

				/// <summary>
				/// <para>Total bytes in memory right now (might not be 100% accurate).</para>
				/// </summary>
				/// <returns>{size_t} Total bytes in memory.</returns>
				size_t bytes_read() const;

				/// <summary>
				/// <para>Reads data from Buffer.</para>
				/// </summary>
				/// <returns>{Buffer} Constant reference to Buffer.</returns>
				const Tools::Buffer& read() const;
			};
		}
	}
}