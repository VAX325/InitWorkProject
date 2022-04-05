#include "P_HEADER.h"
#include "W_WebDispatcher.h"
#include "U_DataParser.h"

void W_WebDispatcher::Handle(WebToolkit::HttpServerContext* context)
{
	std::string urlPath = CoreToolkit::Util::URLDecode(context->requestHeader.resource);
	if (!CoreToolkit::FileUtils::PathValid(urlPath))
		throw WebToolkit::HttpException(WebToolkit::HttpNotFound, "Not found.");

	std::string cwd = U_GetCWD();
	using size = std::basic_string<char, std::char_traits<char>, std::allocator<char>>::size_type;

	if(urlPath.length() > 2)
	{
		//trying to search requested file

		std::string absolutePath = cwd + urlPath;

		auto res = CoreToolkit::FileUtils::CheckPath(absolutePath);
		if (res != CoreToolkit::PathNotExist)
		{
			if (res != CoreToolkit::PathIsDirectory)
			{
				time_t t;
				time(&t);
				t += 86400;
				context->responseHeader.expireTime = t;
				context->ServeFile(absolutePath);
				return;
			}
		}
	}

	WebToolkit::URIDispatcher::Handle(context);
}
