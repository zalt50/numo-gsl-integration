#include "integration_internal.h"

void integration_options_parse(VALUE options, integration_options *parsed,
                               double default_epsabs, double default_epsrel,
                               size_t default_limit) {
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"),
                   rb_intern("limit")};
  VALUE keyword_values[3];

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 3, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
  }

  parsed->epsabs =
      epsabs_value == Qundef ? default_epsabs : NUM2DBL(epsabs_value);
  parsed->epsrel =
      epsrel_value == Qundef ? default_epsrel : NUM2DBL(epsrel_value);
  parsed->limit =
      limit_value == Qundef ? default_limit : NUM2SIZET(limit_value);
}

void integration_options_parse_with_workspace(
    VALUE options, integration_options *parsed, double default_epsabs,
    double default_epsrel, size_t default_limit, VALUE *workspace_value,
    int *limit_given) {
  VALUE epsabs_value = Qundef;
  VALUE epsrel_value = Qundef;
  VALUE limit_value = Qundef;
  ID keywords[] = {rb_intern("epsabs"), rb_intern("epsrel"), rb_intern("limit"),
                   rb_intern("workspace")};
  VALUE keyword_values[4];

  if (!NIL_P(options)) {
    rb_get_kwargs(options, keywords, 0, 4, keyword_values);
    epsabs_value = keyword_values[0];
    epsrel_value = keyword_values[1];
    limit_value = keyword_values[2];
    *workspace_value = keyword_values[3];
  } else {
    *workspace_value = Qundef;
  }

  parsed->epsabs =
      epsabs_value == Qundef ? default_epsabs : NUM2DBL(epsabs_value);
  parsed->epsrel =
      epsrel_value == Qundef ? default_epsrel : NUM2DBL(epsrel_value);
  parsed->limit =
      limit_value == Qundef ? default_limit : NUM2SIZET(limit_value);
  *limit_given = limit_value != Qundef;
}
