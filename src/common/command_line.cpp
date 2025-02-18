// Copyright (c) 2018, Ombre Cryptocurrency Project
// Copyright (c) 2018, Ryo Currency Project
// Portions copyright (c) 2014-2018, The Monero Project
//
// Portions of this file are available under BSD-3 license. Please see ORIGINAL-LICENSE for details
// All rights reserved.
//
// Ombre changes to this code are in public domain. Please note, other licences may apply to the file.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include "command_line.h"
#include "common/i18n.h"
#include "cryptonote_config.h"
#include "string_tools.h"
#include <boost/algorithm/string/compare.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <unordered_set>

#if defined(WIN32)
#include <windows.h>
#include <shellapi.h>
#endif

namespace command_line
{
namespace
{
const char *tr(const char *str)
{
	return i18n_translate(str, "command_line");
}
}

bool is_yes(const std::string &str)
{
	if(str == "y" || str == "Y")
		return true;

	boost::algorithm::is_iequal ignore_case{};
	if(boost::algorithm::equals("yes", str, ignore_case))
		return true;
	if(boost::algorithm::equals(command_line::tr("yes"), str, ignore_case))
		return true;

	return false;
}

bool is_no(const std::string &str)
{
	if(str == "n" || str == "N")
		return true;

	boost::algorithm::is_iequal ignore_case{};
	if(boost::algorithm::equals("no", str, ignore_case))
		return true;
	if(boost::algorithm::equals(command_line::tr("no"), str, ignore_case))
		return true;

	return false;
}

#ifdef WIN32
bool get_windows_args(std::vector<char*>& argptrs)
{
	int nArgs = 0;
	LPWSTR sCmdArgs = GetCommandLineW();
	LPWSTR* szArgs = CommandLineToArgvW(sCmdArgs, &nArgs);

	if(szArgs == nullptr)
		return false;

	size_t iSlen = wcslen(sCmdArgs) * 3 + nArgs; //Guarantees fit for all BMP  and SMP glyphs
	char* strptr = new char[iSlen];
	for(int i = 0; i < nArgs; i++)
	{
		int ret = WideCharToMultiByte(CP_UTF8, 0, szArgs[i], -1, strptr, iSlen, NULL, NULL);
		if(ret <= 0)
		{
			argptrs.clear();
			delete[] strptr;
			return false;
		}

		argptrs.emplace_back(strptr);
		strptr += ret;
		iSlen -= ret;
	}

	return true;
}

void set_console_utf8()
{
	SetConsoleCP(CP_UTF8);
}
#endif

const arg_descriptor<bool> arg_help = {"help", "Produce help message"};
const arg_descriptor<bool> arg_version = {"version", "Output version information"};
}
