NAME        = webserv
CXX         = c++

INCFLAGS    = -Iincludes

CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -pedantic -O3
DEPFLAGS    = -MMD -MP

RM          = rm -f
MAKE        = make
MAKE_FLAGS  += --no-print-directory

SRCDIR      = srcs
SRCS        = srcs/eventloop/EventLoop.cpp srcs/eventloop/Client.cpp \
              srcs/eventloop/Socket.cpp srcs/eventloop/Epoll.cpp \
              srcs/config/Config.cpp srcs/config/LocationContext.cpp srcs/config/ServerContext.cpp srcs/config/utils.cpp \
              srcs/engine/engine.cpp srcs/engine/CgiHandler.cpp srcs/Http/HttpResponse.cpp \
              srcs/Http/HttpRequest.cpp srcs/Http/HttpRequestParser.cpp srcs/main.cpp

OBJS        = $(SRCS:%.cpp=%.o)
DEPS        = $(OBJS:%.o=%.d)

TOTAL       := $(words $(OBJS))
COUNT       = 0

RESET   = \033[0m
RED     = \033[31m
GREEN   = \033[32m
YELLOW  = \033[33m
BLUE    = \033[34m
MAGENTA = \033[35m
CYAN    = \033[36m
WHITE   = \033[37m
L_RED   = \033[91m
L_GREEN = \033[92m
L_BLUE  = \033[94m

all: $(NAME)

$(NAME): $(OBJS)
	@printf "\r$(L_GREEN)[%s] 100%% Building $(NAME)...$(RESET)\n" "$$(printf '%0.s#' $$(seq 1 $(TOTAL)))"
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) $(LDFLAGS)
	@echo "\n$(GREEN)🎉🎉 Build complete: $(NAME) 🎉🎉$(RESET)\n"

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@
	@$(eval COUNT=$(shell echo $$(($(COUNT)+1))))
	@FILLED=$(COUNT); \
	EMPTY=$$(($(TOTAL) - $$FILLED)); \
	BAR="$$(printf '%0.s#' $$(seq 1 $$FILLED))$$(printf '%0.s.' $$(seq 1 $$EMPTY))"; \
	PERCENT=$$(( $$FILLED * 100 / $(TOTAL) )); \
	printf "\r$(L_GREEN)[%s] %d%% Building $(NAME)...$(RESET)" "$$BAR" "$$PERCENT"

clean:
	@echo "$(RED)🧹 Cleaning objects...$(RESET)"
	@$(RM) $(OBJS) $(DEPS)
	@echo "$(L_GREEN)✅ clean done$(RESET)\n"

fclean: clean
	@echo "$(RED)🗑️  Removing binary...$(RESET)"
	@$(RM) $(NAME)
	@echo "$(L_GREEN)✅ fclean done$(RESET)\n"

re:
	$(MAKE) fclean
	$(MAKE) all

run: all
	@$(MAKE) $(MAKE_FLAGS) clean -s
	@./$(NAME) default.conf

test:
	@$(MAKE) $(MAKE_FLAGS) -C unit_test run

docker:
	@echo "$(CYAN)🐳 Setting up Docker environment...$(RESET)"
	docker-compose up -d --build
	@echo "---------------------------------------------------"
	@echo "Webserv Docker environment is ready!"
	@echo "To enter the container, run:"
	@echo "  docker exec -it \$$(docker-compose ps -q webserv) bash"
	@echo "---------------------------------------------------"

help:
	@echo "$(CYAN)Available targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)      : Build the webserv executable (default)"
	@echo "  $(GREEN)clean$(RESET)    : Remove object files"
	@echo "  $(GREEN)fclean$(RESET)   : Remove object files and the executable"
	@echo "  $(GREEN)re$(RESET)       : Rebuild the project from scratch"
	@echo "  $(GREEN)run$(RESET)      : Build and run the webserv with default.conf"
	@echo "  $(GREEN)test$(RESET)     : Run unit tests"
	@echo "  $(GREEN)docker$(RESET)   : Start Docker environment and print command to enter it"
	@echo "  $(GREEN)help$(RESET)     : Show this help message"

-include $(DEPS)

.PHONY: all clean fclean re run test format docker help
