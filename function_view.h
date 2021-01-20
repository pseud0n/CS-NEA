class FunctionView {
private:
	//using StoredT = std::optional<>
public:
	CppFunction *viewing = 0;
	//std::optional<ExternalObject> applied_value = std::nullopt;
	ExternalObject *applied_value = 0;
	// object need to be default-constructible

	FunctionView();

	FunctionView(
		CppFunction* viewing,
		ExternalObject& applied_value);
	
	~FunctionView();

	ExternalObject operator ()(const std::vector<ExternalObject>&);
};