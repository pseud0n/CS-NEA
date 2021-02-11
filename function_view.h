class FunctionView {
private:
	//using StoredT = std::optional<>
public:
	//CppFunction *viewing = 0;
	ExternalObject viewing = nullptr;
	//std::optional<ExternalObject> applied_value = std::nullopt;
	ExternalObject applied_value = nullptr;
	// object need to be default-constructible

	FunctionView();

	FunctionView(ExternalObject&, ExternalObject&);
	
	~FunctionView();

	ExternalObject operator ()();
	ExternalObject operator ()(std::vector<ExternalObject>&);

};