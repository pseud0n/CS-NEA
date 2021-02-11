FunctionView::FunctionView() {
}

FunctionView::FunctionView(
	ExternalObject& viewing,
	ExternalObject& applied_value)
	: viewing(viewing), applied_value(applied_value) {
}

ExternalObject FunctionView::operator ()(std::vector<ExternalObject>& args) {
//	if (applied_value) {
		std::vector<ExternalObject> args_with_extra(1 + args.size());
		args_with_extra[0] = applied_value;
		std::copy(args.begin(), args.end(), args_with_extra.begin() + 1);
		return viewing(args_with_extra);
//	}
//	return (*viewing)(args);
}


ExternalObject FunctionView::operator ()() {
	std::vector<ExternalObject> applied_as_vec;
	applied_as_vec.emplace_back(applied_value);
	return viewing(applied_as_vec);
	// capacity = size = 1, no memory wasted

}


FunctionView::~FunctionView() {
}