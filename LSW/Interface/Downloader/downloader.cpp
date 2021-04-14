#include "downloader.h"

namespace LSW {
	namespace v5 {
		namespace Interface {

			bool Downloader::_get(const std::string& url, std::function<bool(void)> run)
			{
#ifdef UNICODE
				HINTERNET connect = InternetOpen(L"LSW Downloader V5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#else
				HINTERNET connect = InternetOpen("LSW Downloader V5.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
#endif
				if (!connect) return false;
				HINTERNET OpenAddress = InternetOpenUrlA(connect, url.c_str(), NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_KEEP_CONNECTION, 0);
				if (!OpenAddress) return false;


				char DataReceived[download::max_block_size];
				DWORD NumberOfBytesRead = 0;
				TotalBytesRead = 0;

				buf->clear();

				buf->resize(download::max_block_size); // set maximum size

				while (InternetReadFile(OpenAddress, DataReceived, download::max_block_size, &NumberOfBytesRead) && NumberOfBytesRead && run()) {
					buf->resize(TotalBytesRead + NumberOfBytesRead);
					memcpy_s(buf->data() + TotalBytesRead, NumberOfBytesRead, DataReceived, NumberOfBytesRead);
					TotalBytesRead += NumberOfBytesRead;
				}
				buf->resize(TotalBytesRead);
				buf->shrink_to_fit();

				InternetCloseHandle(OpenAddress);
				InternetCloseHandle(connect);
				return run();
			}

			Downloader::~Downloader()
			{
				thr.join();
			}
			
			bool Downloader::get(const std::string& url)
			{
				return _get(url, [] {return true; });
			}
			
			Tools::Future<std::shared_ptr<Tools::Buffer>> Downloader::get_async(const std::string& url)
			{
				if (thr.running()) return Tools::fake_future<std::shared_ptr<Tools::Buffer>>(std::shared_ptr<Tools::Buffer>());
				thr.join();
				thr.set([&, url](Tools::boolThreadF f) { _get(url, f); return buf; });
				return thr.start();
			}
			
			size_t Downloader::bytes_read() const
			{
				return TotalBytesRead;
			}
			
			const Tools::Buffer& Downloader::read() const
			{
				return *buf.get();
			}

			// implementation

			template Tools::Future<std::shared_ptr<std::string>>;
			template Tools::SuperThread<std::shared_ptr<std::string>>;

		}
	}
}
