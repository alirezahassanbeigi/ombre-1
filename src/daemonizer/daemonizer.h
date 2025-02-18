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

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace daemonizer
{
void init_options(
	boost::program_options::options_description &hidden_options, boost::program_options::options_description &normal_options);

boost::filesystem::path get_default_data_dir();

boost::filesystem::path get_relative_path_base(
	boost::program_options::variables_map const &vm);

/**
   * @arg create_before_detach - this indicates that the daemon should be
   * created before the fork, giving it a chance to report initialization
   * errors.  At the time of this writing, this is not possible in the primary
   * daemon (likely due to the size of the blockchain in memory).
   */
template <typename T_executor>
bool daemonize(
	int argc, char* argv[], T_executor &&executor // universal ref
	,
	boost::program_options::variables_map const &vm);
}

#ifdef WIN32
#include "daemonizer/windows_daemonizer.inl"
#else
#include "daemonizer/posix_daemonizer.inl"
#endif
