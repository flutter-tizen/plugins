#include "convert.h"

#include <json-glib/json-glib.h>

#include "log.h"

bool Convert::GetBound(const std::string& bound, double& south, double& west,
                       double& north, double& east) {
  JsonParser* parser = json_parser_new();
  GError* error = nullptr;

  json_parser_load_from_data(parser, bound.c_str(), bound.size(), &error);
  if (error) {
    LOG_ERROR("Error in parsing json data : %s", error->message);
    g_error_free(error);
    g_object_unref(parser);
    return false;
  }

  JsonReader* reader = json_reader_new(json_parser_get_root(parser));
  char** members = json_reader_list_members(reader);

  for (int i = 0; members[i] != 0; i++) {
    json_reader_read_member(reader, members[i]);
    double value = json_reader_get_double_value(reader);
    json_reader_end_member(reader);

    LOG_DEBUG("key : %s, val: %f", members[i], value);

    if (strcmp(members[i], "south") == 0) {
      south = value;
    } else if (strcmp(members[i], "west") == 0) {
      west = value;
    } else if (strcmp(members[i], "north") == 0) {
      north = value;
    } else if (strcmp(members[i], "east") == 0) {
      east = value;
    } else {
      LOG_ERROR("Error in parsing value");
    }
  }

  g_strfreev(members);
  g_object_unref(reader);
  g_object_unref(parser);

  return true;
}
