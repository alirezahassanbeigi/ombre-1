// Copyright (c) 2018, Ombre Cryptocurrency Project
// Copyright (c) 2018, Ryo Currency Project
// Portions copyright (c) 2014-2018, The Monero Project
//
// Portions of this file are available under BSD-3 license. Please see ORIGINAL-LICENSE for details
// All rights reserved.
//
// Ombre changes to this code are in public domain. Please note, other licences may apply to the file.
// Ryo changes to this code are in public domain. Please note, other licences may apply to the file.
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

#include "wallet/api/wallet2_api.h"
#include <ctime>
#include <string>

namespace Ryo
{

class TransactionHistoryImpl;

class TransactionInfoImpl : public TransactionInfo
{
  public:
	TransactionInfoImpl();
	~TransactionInfoImpl();
	//! in/out
	virtual int direction() const;
	//! true if hold
	virtual bool isPending() const;
	virtual bool isFailed() const;
	virtual uint64_t amount() const;
	//! always 0 for incoming txes
	virtual uint64_t fee() const;
	virtual uint64_t blockHeight() const;
	virtual std::set<uint32_t> subaddrIndex() const;
	virtual uint32_t subaddrAccount() const;
	virtual std::string label() const;

	virtual std::string hash() const;
	virtual std::time_t timestamp() const;
	virtual std::string paymentId() const;
	virtual const std::vector<Transfer> &transfers() const;
	virtual uint64_t confirmations() const;
	virtual uint64_t unlockTime() const;

  private:
	int m_direction;
	bool m_pending;
	bool m_failed;
	uint64_t m_amount;
	uint64_t m_fee;
	uint64_t m_blockheight;
	std::set<uint32_t> m_subaddrIndex; // always unique index for incoming transfers; can be multiple indices for outgoing transfers
	uint32_t m_subaddrAccount;
	std::string m_label;
	std::string m_hash;
	std::time_t m_timestamp;
	std::string m_paymentid;
	std::vector<Transfer> m_transfers;
	uint64_t m_confirmations;
	uint64_t m_unlock_time;

	friend class TransactionHistoryImpl;
};

} // namespace
