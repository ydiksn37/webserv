NAME        = webserv
CXX         = c++

INCFLAGS    = -I./includes

CXXFLAGS    = -Wall -Wextra -Werror -std=c++98 -pedantic -O3
DEPFLAGS    = -MMD -MP

UNAME_S     := $(shell uname -s)

RM          = rm -f
MAKE        = make
MAKE_FLAGS  += --no-print-directory

SRCDIR      = srcs
SRCS        = $(shell find $(SRCDIR) -type f -name "*.cpp")

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

re: fclean all

run: all
	@$(MAKE) $(MAKE_FLAGS) clean -s
	@./$(NAME) default.conf

format:
	@echo "$(YELLOW)🎨 Formatting code...$(RESET)"
	@find $(SRCDIR) -type f \( -name "*.cpp" -o -name "*.hpp" \) | xargs clang-format -i
	@echo "$(L_GREEN)✨ Formatting complete!$(RESET)\n"

-include $(DEPS)

.PHONY: all clean fclean re run format
