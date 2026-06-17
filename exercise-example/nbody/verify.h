#define VERIFY(expr)                                 \
  do {                                               \
    if (not (expr)) throw "failure at: `" #expr "`"; \
  } while (false)

