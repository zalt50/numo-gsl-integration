# GSL error handling

`numo-gsl-integration` defines `Numo::GSL::GslError` as a subclass of `StandardError` and installs a process-wide GSL error handler when the native extension is loaded.  
Errors reported by GSL are raised directly as `Numo::GSL::GslError`. The message contains the GSL source file, line number, reason, and numeric error code:

```text
file:line: reason (GSL error code)
```

`numo-gsl-integration` does not convert integration return statuses into separate exceptions.  
Temporary workspaces used by QAG, QAGS, QAGP, QAGI, QAGIU, QAGIL, QAWC, QAWS, QAWO, QAWF, CQUAD, and Romberg are released when a call completes normally or raises an exception. Reusable `Workspace` and `CQUADWorkspace` objects supplied by the caller remain open and are made available for the next call. QAWS, QAWO, and QAWF release only temporary weight tables, GLFIXED releases its Gauss–Legendre table, and QAGP releases its temporary copy of the point array.  
Invalid argument types raise `TypeError`. Invalid values, counts, or ordering raise `ArgumentError`. Exceptions raised by the integrand are propagated unchanged.  
The GSL error handler is shared by the entire process. Loading this gem replaces the previously registered handler. If another GSL binding is loaded afterward, that binding may replace this gem's handler in turn.
