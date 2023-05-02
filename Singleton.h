#pragma once

template<typename T>
class CSingletonImpl
{
public:
	static T& Instance();

	CSingletonImpl(const CSingletonImpl&) = delete;
	CSingletonImpl& operator= (const CSingletonImpl) = delete;

protected:
	struct token {};
	CSingletonImpl() {}
};

template<typename T>
T& CSingletonImpl<T>::Instance()
{
	static T instance;
	return instance;
}
