/*
* Copyright 2018 NXP.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* Redistributions in binary form must reproduce the above copyright notice, this
* list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* Neither the name of the NXP Semiconductor nor the names of its
* contributors may be used to endorse or promote products derived from this
* software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*/
#include <string>
#include <vector>
#include "libcomm.h"
#include "libuuu.h"
#include "trans.h"
#include <string.h>

#pragma once

using namespace std;

class HIDReport
{
	int m_size_in;
	int m_size_out;
	int m_size_payload;
public:
	TransBase * m_pdev;
	vector<uint8_t> m_out_buff;
	void init()
	{
		m_size_in = 64; 
		m_size_out = 1024;
		m_size_payload = 1;
		m_out_buff.resize(m_size_out + m_size_payload);
	}
	HIDReport()
	{
		init();
	}

	HIDReport(TransBase *trans)
	{
		init();
		m_pdev = trans;
	}

	int read(vector<uint8_t> &buff)
	{
		size_t rs;
		if (buff.size() < m_size_in + m_size_payload)
		{
			set_last_err_string("buffer to small to get a package");
			return -1;
		}
		int ret = m_pdev->read(buff.data(), m_size_in + m_size_payload, &rs);
		if (ret)
			return ret;

		return ret;
	}

	int write(void *p, size_t sz, uint8_t report_id)
	{
		size_t off;
		uuu_notify nf;

		uint8_t *buff = (uint8_t *)p;

		nf.type = uuu_notify::NOTIFY_TRANS_SIZE;
		nf.index = sz;
		call_notify(nf);

		for (off = 0; off < sz; off += m_size_out)
		{
			m_out_buff[0] = report_id;

			size_t s = sz - off;
			if (s > m_size_out)
				s = m_size_out;

			memcpy(m_out_buff.data() + m_size_payload, buff + off, s);

			int ret = m_pdev->write(m_out_buff.data(), report_id == 1? s + m_size_payload: m_size_out + m_size_payload);
			
			if ( ret < 0)
				return -1;

			if (off % 0x1F == 0)
			{
				uuu_notify nf;
				nf.type = uuu_notify::NOTIFY_TRANS_POS;
				nf.index = off;
				call_notify(nf);
			}
		}
		return 0;
	}

	int write(vector<uint8_t> &buff, uint8_t report_id)
	{
		return write(buff.data(), buff.size(), report_id);
	}
		
};