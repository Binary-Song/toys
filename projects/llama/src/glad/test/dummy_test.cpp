// #include <glad/glad.h>

// #include <GLFW/glfw3.h>
// #include <gtest/gtest.h>
// #include <iostream>

// void framebuffer_size_callback(GLFWwindow *window, int width, int height);
// void processInput(GLFWwindow *window);

// // settings
// const unsigned int SCR_WIDTH = 800;
// const unsigned int SCR_HEIGHT = 600;

// const char *vertexShaderSource = R"~~~(#version 330 core
// layout (location = 0) in vec3 aPos;
// void main()
// {
//    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
// }
// )~~~";

// const char *fragmentShaderSource = R"~~~(#version 330 core
// out vec4 FragColor;
// void main()
// {
//    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
// }
// )~~~";

// TEST(GL, A)
// {
//     // glfw: initialize and configure
//     // ------------------------------
//     glfwInit();
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//     glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//     glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

// #ifdef __APPLE__
//     glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
// #endif

//     // glfw window creation
//     // --------------------
//     GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//     if (window == NULL)
//     {
//         std::cerr << "Failed to create GLFW window" << std::endl;
//         glfwTerminate();
//         throw -1;
//     }
//     glfwMakeContextCurrent(window);
//     glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

//     // glad: load all OpenGL function pointers
//     // ---------------------------------------
//     if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//     {
//         std::cerr << "Failed to initialize GLAD" << std::endl;
//         throw -1;
//     }

//     // build and compile our shader program
//     // ------------------------------------
//     // vertex shader
//     unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
//     glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
//     glCompileShader(vertexShader);
//     // check for shader compile errors
//     int success;
//     char infoLog[512];
//     glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
//     if (!success)
//     {
//         glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
//         std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
//         throw -1;
//     }
//     // fragment shader
//     unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
//     glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
//     glCompileShader(fragmentShader);
//     // check for shader compile errors
//     glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
//     if (!success)
//     {
//         glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
//         std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
//         throw -1;
//     }
//     // link shaders
//     unsigned int shaderProgram = glCreateProgram();
//     glAttachShader(shaderProgram, vertexShader);
//     glAttachShader(shaderProgram, fragmentShader);
//     glLinkProgram(shaderProgram);
//     // check for linking errors
//     glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
//     if (!success)
//     {
//         glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
//         std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//         throw -1;
//     }
//     glDeleteShader(vertexShader);
//     glDeleteShader(fragmentShader);

//     // set up vertex data (and buffer(s)) and configure vertex attributes
//     // ------------------------------------------------------------------
//     float vertices[] = {
//         0.5f,  0.5f,  0.0f, // top right
//         0.5f,  -0.5f, 0.0f, // bottom right
//         -0.5f, -0.5f, 0.0f, // bottom left
//         -0.5f, 0.5f,  0.0f  // top left
//     };
//     unsigned int indices[] = {
//         // note that we start from 0!
//         0, 1, 3, // first triangle
//         1, 2, 3  // second triangle
//     };

//     unsigned int VBO, VAO, EBO;
//     glGenVertexArrays(/*num=*/1, &VAO);
//     glGenBuffers(1, &VBO);
//     glGenBuffers(1, &EBO);

//     // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
//     glBindVertexArray(VAO);

//     // set buffer as 'current'
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     // allocate and copy buffer data
//     glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//     // set vertex attributes
//     //   this tells vertex shader where to find attribute 0
//     glVertexAttribPointer(/* index */ 0, /*size*/ 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

//     // enable attributes
//     glEnableVertexAttribArray(0);
//     // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound
//     // vertex buffer object so afterwards we can safely unbind
//     glBindBuffer(GL_ARRAY_BUFFER, 0);

//     // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens.
//     // Modifying other VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs)
//     // when it's not directly necessary.
//     glBindVertexArray(0);

//     // uncomment this call to draw in wireframe polygons.
//     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//     // render loop
//     // -----------
//     while (!glfwWindowShouldClose(window))
//     {
//         // input
//         // -----
//         processInput(window);

//         // render
//         // ------
//         glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);

//         // draw our first triangle
//         glUseProgram(shaderProgram);
//         glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll
//                                 // do so to keep things a bit more organized
//         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//         // glBindVertexArray(0); // no need to unbind it every time

//         // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
//         // -------------------------------------------------------------------------------
//         glfwSwapBuffers(window);
//         glfwPollEvents();
//     }

//     // optional: de-allocate all resources once they've outlived their purpose:
//     // ------------------------------------------------------------------------
//     glDeleteVertexArrays(1, &VAO);
//     glDeleteBuffers(1, &VBO);
//     glDeleteProgram(shaderProgram);

//     // glfw: terminate, clearing all previously allocated GLFW resources.
//     // ------------------------------------------------------------------
//     glfwTerminate();
// }

// // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// // ---------------------------------------------------------------------------------------------------------
// void processInput(GLFWwindow *window)
// {
//     if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//         glfwSetWindowShouldClose(window, true);
// }

// // glfw: whenever the window size changed (by OS or user resize) this callback function executes
// // ---------------------------------------------------------------------------------------------
// void framebuffer_size_callback(GLFWwindow *window, int width, int height)
// {
//     // make sure the viewport matches the new window dimensions; note that width and
//     // height will be significantly larger than specified on retina displays.
//     glViewport(0, 0, width, height);
// }