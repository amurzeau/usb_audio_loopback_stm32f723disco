#pragma once

#include "OscReadOnlyVariable.h"
#include <memory>

template<typename T> class OscVariable : public OscReadOnlyVariable<T> {
public:
	using typename OscReadOnlyVariable<T>::readonly_type;
	OscVariable(OscContainer* parent,
	            std::string_view name,
	            readonly_type initialValue = {},
	            bool fixedSize = false) noexcept;

	using OscReadOnlyVariable<T>::operator=;
	OscVariable& operator=(const OscVariable<T>& v);

	void execute(const std::vector<OscArgument>& arguments) override;

	void setIncrementAmount(T amount) { incrementAmount = amount; }

	std::string getAsString() const override;

private:
	T incrementAmount;
	std::vector<std::unique_ptr<OscEndpoint>> subEndpoint;
	bool fixedSize;
};

EXPLICIT_INSTANCIATE_OSC_VARIABLE(extern template, OscVariable)
