llama_module(fso SHARED ENABLE_RTTI) # cryptopp 要求 RTTI :(
llama_link(fso PRIVATE cryptopp::cryptopp)
llama_link(fso PUBLIC base)
