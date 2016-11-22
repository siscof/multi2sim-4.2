/*********************************************************************************
*  Copyright (c) 2010-2011, Elliott Cooper-Balis
*                             Paul Rosenfeld
*                             Bruce Jacob
*                             University of Maryland
*                             dramninjas [at] gmail [dot] com
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright notice,
*        this list of conditions and the following disclaimer.
*
*     * Redistributions in binary form must reproduce the above copyright notice,
*        this list of conditions and the following disclaimer in the documentation
*        and/or other materials provided with the distribution.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
*  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
*  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
*  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*********************************************************************************/



#include <stdint.h> // uint64_t

#ifndef CALLBACK_H
#define CALLBACK_H

namespace DRAMSim
{

template <typename ReturnT, typename... ParamsT>
class CallbackBase
{
public:
	virtual ~CallbackBase() = 0;
	virtual ReturnT operator()(ParamsT...) = 0;
	virtual CallbackBase* clone() const = 0;
};

template <typename Return, typename... Params>
DRAMSim::CallbackBase<Return,Params...>::~CallbackBase() {}



template <typename ConsumerT, typename ReturnT, typename... ParamsT>
class Callback: public CallbackBase<ReturnT,ParamsT...>
{
private:
	typedef ReturnT (ConsumerT::*PtrMember)(ParamsT...);

public:
	Callback( ConsumerT* const object, PtrMember member) :
			object(object), member(member)
	{
	}

	Callback(const Callback<ConsumerT,ReturnT,ParamsT...> &e) :
			object(e.object), member(e.member)
	{
	}

	ReturnT operator()(ParamsT... params)
	{
		return (const_cast<ConsumerT*>(object)->*member)
				(std::forward<ParamsT>(params)...);
	}

	virtual CallbackBase<ReturnT,ParamsT...>* clone() const
	{
		return new Callback(static_cast<const Callback&>(*this)); // call the copy ctor.
	}

private:

	ConsumerT* const object;
	const PtrMember member;
};


/* Callback which wraps a simple function */
template <typename ReturnT, typename... ParamsT>
class SimpleCallback: public CallbackBase<ReturnT,ParamsT...>
{
	typedef ReturnT(*FuncPtr_t)(ParamsT...);

public:
	SimpleCallback(FuncPtr_t func_ptr) :
			func_ptr(func_ptr) {}

	ReturnT operator()(ParamsT... params)
	{
		return (*func_ptr)(std::forward<ParamsT>(params)...);
	}

	virtual CallbackBase<ReturnT,ParamsT...>* clone() const
	{
		return new SimpleCallback(static_cast<const SimpleCallback&>(*this)); // call the copy ctor.
	}

private:
	SimpleCallback();
	FuncPtr_t func_ptr;
};


/* Callback which wraps a simple function with a payload, which is passed as first argument */
template <typename PayloadT, typename ReturnT, typename... ParamsT>
class SimplePayloadedCallback: public CallbackBase<ReturnT,ParamsT...>
{
	typedef ReturnT(*FuncPtr_t)(PayloadT,ParamsT...);

public:
	SimplePayloadedCallback(FuncPtr_t func_ptr, PayloadT payload) :
			func_ptr(func_ptr), payload(payload) {}

	ReturnT operator()(ParamsT... params)
	{
		return (*func_ptr)(payload, std::forward<ParamsT>(params)...);
	}

	virtual CallbackBase<ReturnT,ParamsT...>* clone() const
	{
		return new SimplePayloadedCallback(static_cast<const SimplePayloadedCallback&>(*this)); // call the copy ctor.
	}

private:
	SimplePayloadedCallback();

	FuncPtr_t func_ptr;
	PayloadT payload;
};


typedef CallbackBase <void, unsigned, uint64_t, uint64_t> TransactionCompleteCB;
} // namespace DRAMSim

#endif
