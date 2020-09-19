#include "db_queries_setup.h"
#include "db_tables.h"
#include "library_tables.h"

#include "gtest/gtest.h"
#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>
#include <numeric>

#include <assert.h>


LibraryDBTables adb;

TEST(db_where_tree, model_info) {
  book b;
  /* empty(for 'select' it is all data) */
  b.initialized = 0;
  std::unique_ptr<db_where_tree> wt(adb.InitWhereTree(b));
  EXPECT_TRUE(wt == nullptr);
  b.id = 101;
  b.initialized = b.f_id;
  wt.reset(adb.InitWhereTree(b));
  EXPECT_EQ(trim_str(wt->GetString()), db_condition_node::DataToStr(
      db_type::type_int, TABLE_FIELD_NAME(BOOK_ID), std::to_string(b.id)));
  b.title = "ABC";
  b.initialized |= b.f_title;
  wt.reset(adb.InitWhereTree(b));
  /* строка стандартными методами */
  std::string where_str1 = db_condition_node::DataToStr(
      db_type::type_int, TABLE_FIELD_NAME(BOOK_ID), std::to_string(b.id));
  where_str1 += " AND ";
  where_str1 += db_condition_node::DataToStr(
      db_type::type_char_array, TABLE_FIELD_NAME(BOOK_TITLE), b.title);
  /* строка в обычном представлении */
  std::string where_str2 = std::string(TABLE_FIELD_NAME(BOOK_ID))+ " = 101 AND " +
      TABLE_FIELD_NAME(BOOK_TITLE) + " = 'ABC'";
  EXPECT_EQ(trim_str(wt->GetString()), where_str1);
  EXPECT_EQ(trim_str(wt->GetString()), where_str2);
}