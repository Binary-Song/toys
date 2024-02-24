set(CMAKE_AUTOMOC ON)
llama_module(base SHARED)
llama_link(base PUBLIC Qt5::Widgets cryptopp::cryptopp)
