llama_module(fso SHARED)
llama_link(fso PRIVATE cryptopp::cryptopp)
llama_link(fso PUBLIC base)
