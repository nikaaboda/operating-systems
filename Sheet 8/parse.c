#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "./nxjson.h"

int parse(char *json, quiz_t *quiz) {
    const nx_json* parsedJson = nx_json_parse(json, 0);
    char *text =(char *) nx_json_get(parsedJson, "text")->text_value;
    int number = nx_json_get(parsedJson, "number")->num.u_value;
    quiz->text = text;
    quiz->number = number;
    return 0;
}