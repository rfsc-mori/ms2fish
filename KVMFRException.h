#ifndef KVMFREXCEPTION_HPP
#define KVMFREXCEPTION_HPP

#include <stdexcept>

class KVMFRException : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

class KVMFRInvalidHeaderException : public KVMFRException
{
	using KVMFRException::KVMFRException;
};

class KVMFRUnsupportedFrameTypeException : public KVMFRException
{
	using KVMFRException::KVMFRException;
};

#endif // KVMFREXCEPTION_HPP
