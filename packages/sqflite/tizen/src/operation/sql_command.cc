#include <list>

#include "ctype.h"
#include "sstream"
#include "string"

template <typename Base, typename T>
inline bool instanceof (const T* ptr) {
  return dynamic_cast<const Base*>(ptr) != nullptr;
}

class SqlCommand {
  std::string sql;
  std::list<void*> rawArguments;

  SqlCommand(std::string aSql, std::list<void*> aRawArguments) {
    sql = aSql;
    rawArguments = aRawArguments;
  }

 public:
  std::string getSql() { return sql; }

  SqlCommand sanitizeForQuery() {
    if (rawArguments.size() == 0) {
      return *this;
    }
    std::stringstream sanitizeSqlSb;
    std::list<void*> sanitizeArguments;
    int count = 0;
    int argumentIndex = 0;
    int sqlLength = sql.length();
    for (int i = 0; i < sqlLength; i++) {
      char ch = sql.at(i);
      if (ch == '?') {
        // If it is followed by a number
        // it is an indexed param, cancel our weird conversion
        if ((i + 1 < sqlLength) && isdigit(sql.at(i + 1))) {
          return *this;
        }
        count++;
        // no match, return the same
        if (argumentIndex >= rawArguments.size()) {
          return *this;
        }
        std::list<void*>::iterator it = rawArguments.begin();
        std::advance(it, ++argumentIndex);
        void* argument = *it;
        if (instanceof <int>(argument)) {
          int* casted = static_cast<int*>(argument);
          sanitizeSqlSb << std::to_string(*casted);
          continue;
        } else if (instanceof <long>(argument)) {
          long* casted = static_cast<long*>(argument);
          sanitizeSqlSb << std::to_string(*casted);
          continue;
        } else {
          // Let the other args as is
          sanitizeArguments.push_front(argument);
        }
      }
      // Simply append the existing
      sanitizeSqlSb << ch;
    }
    // no match (there might be an extra ? somwhere), return the same
    if (count != rawArguments.size()) {
      return *this;
    }
    return SqlCommand(sanitizeSqlSb.str(), sanitizeArguments);
  }
  std::list<void*> getSqlArguments() { return _getSqlArguments(rawArguments); }

  std::list<std::string> getQuerySqlArguments() {
    return getQuerySqlArguments(rawArguments);
  }

  std::list<void*> getRawSqlArguments() { return rawArguments; }

  int hashCode() { return std::hash<std::string>()(sql); }

  bool equals(void* obj) {
    if (instanceof <SqlCommand>(obj)) {
      SqlCommand* o = static_cast<SqlCommand*>(obj);
      if (sql != (*o).sql) {
        return false;
      }

      if (rawArguments.size() != (*o).rawArguments.size()) {
        return false;
      }
      for (auto const& arg : rawArguments) {
        // special blob handling
        if (arg instanceof byte[] && o.rawArguments.get(i) instanceof byte[]) {
          if (!Arrays.equals((byte[])rawArguments.get(i),
                             (byte[])o.rawArguments.get(i))) {
            return false;
          }
        } else {
          if (!rawArguments.get(i).equals(o.rawArguments.get(i))) {
            return false;
          }
        }
      }
      return true;
    }
    return false;
  }

 private:
  // Handle list of int as byte[]
  static void* toValue(void* value) {
    if (value == NULL) {
      return NULL;
    } else {
      //   if (Debug.EXTRA_LOGV) {
      //     Log.d(TAG, "arg " + value.getClass().getCanonicalName() + " " +
      //                    toString(value));
      //   }
      // Assume a list is a blog
      if (instanceof <std::list<int>>(value) t =
                         dynamic_cast<std::list<int>*>(value)) {
        std::list<int> list = *t;
        unsigned char blob[];
        for (int i = 0; i < list.size(); i++) {
          blob[i] = (byte)(int)list.get(i);
        }
        value = blob;
      }
      //   if (Debug.EXTRA_LOGV) {
      //     Log.d(TAG, "arg " + value.getClass().getCanonicalName() + " " +
      //                    toString(value));
      //   }
      return value;
    }
  }

  // Only sanitize if the parameter count matches the argument count
  // For integer value replace ? with the actual value directly
  // to workaround an issue with references

  // Query only accept string arguments
  // so should not have byte[]
  std::list<std::string> getQuerySqlArguments(std::list<void*> rawArguments) {
    return getStringQuerySqlArguments(rawArguments).toArray(new String[0]);
  }

  std::list<void*> _getSqlArguments(std::list<void*> rawArguments) {
    std::list<void*> fixedArguments;
    int rawArgumentsLength = rawArguments.length();
    for (int i = 0; i < rawArgumentsLength; i++) {
      fixedArguments.push_front(toValue(rawArguments));
    }
    return fixedArguments.toArray(new Object[0]);
  }

  // Query only accept string arguments
  std::list<std::string> getStringQuerySqlArguments(
      std::list<void*>* rawArguments) {
    std::list<std::string> stringArguments;
    if (rawArguments != NULL) {
      for (Object rawArgument : rawArguments) {
        stringArguments.add(toString(rawArgument));
      }
    }
    return stringArguments;
  }

  // Convert a value to a string
  // especially byte[]
  static std::string toString(void* value) {
    if (value == NULL) {
      return NULL;
    } else if (unsigned char* v[] = dynamic_cast<unsigned char*>(value)) {
      std::list<int> list = new ArrayList<>();
      for (byte _byte : (byte[])value) {
        list.add((int)_byte);
      }
      return list.toString();
    } else if (value instanceof Map) {
      @SuppressWarnings("unchecked") Map<Object, Object> mapValue =
          (Map<Object, Object>)value;
      return fixMap(mapValue).toString();
    } else {
      return value.toString();
    }
  }

  static std::map<> Map<String, Object> fixMap(Map<Object, Object> map) {
    Map<String, Object> newMap = new HashMap<>();
    for (Map.Entry<Object, Object> entry : map.entrySet()) {
      Object value = entry.getValue();
      if (value instanceof Map) {
        @SuppressWarnings("unchecked") Map<Object, Object> mapValue =
            (Map<Object, Object>)value;
        value = fixMap(mapValue);
      } else {
        value = toString(value);
      }
      newMap.put(toString(entry.getKey()), value);
    }
    return newMap;
  }

  std::string toString() {
    return sql + ((rawArguments == null || rawArguments.isEmpty())
                      ? ""
                      : (" " + getStringQuerySqlArguments(rawArguments)));
  }

  // As expected by execSQL
}
