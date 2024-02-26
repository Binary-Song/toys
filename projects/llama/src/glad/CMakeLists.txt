if(llama_feature_gui)
	llama_module(glad STATIC)
	llama_link(glad PUBLIC OpenGL::GL glfw)
endif()