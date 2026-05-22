HTTP_TEST   = run_http_tests
CONFIG_TEST = run_config_tests
ENGINE_TEST = run_engine_tests

CXX         = c++
CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -pedantic

ROOTDIR     = ..
INCFLAGS    = -I. \
              -I$(ROOTDIR)/includes \
              -I$(ROOTDIR)/srcs/config \
              -I$(ROOTDIR)/srcs/Http \
              -I$(ROOTDIR)/srcs/eventloop \
              -I$(ROOTDIR)/srcs/engine

COMMON_SRCS = $(ROOTDIR)/srcs/Http/HttpRequest.cpp \
              $(ROOTDIR)/srcs/config/Config.cpp \
              $(ROOTDIR)/srcs/config/ServerContext.cpp \
              $(ROOTDIR)/srcs/config/LocationContext.cpp \
              $(ROOTDIR)/srcs/config/utils.cpp

HTTP_SRCS   = http_request_test.cpp $(COMMON_SRCS)
CONFIG_SRCS = config_test.cpp $(COMMON_SRCS)
ENGINE_SRCS = engine_test.cpp \
              $(COMMON_SRCS) \
              $(ROOTDIR)/srcs/Http/HttpResponse.cpp \
              $(ROOTDIR)/srcs/engine/engine.cpp

HTTP_OBJS   = $(HTTP_SRCS:.cpp=.o)
CONFIG_OBJS = $(CONFIG_SRCS:.cpp=.o)
ENGINE_OBJS = $(ENGINE_SRCS:.cpp=.o)

RESET   = \033[0m
GREEN   = \033[32m
RED     = \033[31m

all: $(HTTP_TEST) $(CONFIG_TEST) $(ENGINE_TEST)

$(HTTP_TEST): $(HTTP_OBJS)
	@$(CXX) $(CXXFLAGS) $(HTTP_OBJS) -o $(HTTP_TEST)
	@echo "$(GREEN)Build complete: $(HTTP_TEST)$(RESET)"

$(CONFIG_TEST): $(CONFIG_OBJS)
	@$(CXX) $(CXXFLAGS) $(CONFIG_OBJS) -o $(CONFIG_TEST)
	@echo "$(GREEN)Build complete: $(CONFIG_TEST)$(RESET)"

$(ENGINE_TEST): $(ENGINE_OBJS)
	@$(CXX) $(CXXFLAGS) $(ENGINE_OBJS) -o $(ENGINE_TEST)
	@echo "$(GREEN)Build complete: $(ENGINE_TEST)$(RESET)"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCFLAGS) -c $< -o $@

run: all
	@./$(HTTP_TEST)
	@./$(CONFIG_TEST)
	@./$(ENGINE_TEST)

clean:
	@rm -f $(HTTP_OBJS) $(CONFIG_OBJS) $(ENGINE_OBJS)

fclean: clean
	@rm -f $(HTTP_TEST) $(CONFIG_TEST) $(ENGINE_TEST)

re: fclean all

.PHONY: all run clean fclean re
