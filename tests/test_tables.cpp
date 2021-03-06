#include "asp_db/db_connection.h"
#include "asp_db/db_connection_manager.h"
#include "library_structs.h"
#include "library_tables.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <numeric>

#include <assert.h>

using namespace asp_db;
LibraryDBTables adb_;

/**
 * \brief Тестинг работы с БД
 * */
class DatabaseTablesTest : public ::testing::Test {
 protected:
  DatabaseTablesTest() : dbm_(&adb_) {
    db_parameters db_par;
    db_par.is_dry_run = false;
    db_par.supplier = db_client::POSTGRESQL;
    db_par.name = "labibliotecadebabel";
    db_par.username = "jorge";
    db_par.password = "my_pass";
    db_par.host = "127.0.0.1";
    db_par.port = 5432;
    EXPECT_TRUE(is_status_aval(dbm_.ResetConnectionParameters(db_par)));
    if (dbm_.GetError()) {
      std::cerr << dbm_.GetError();
      EXPECT_TRUE(false);
    }
  }

  ~DatabaseTablesTest() {}

 protected:
  /**
   * \brief Менеджер подключения к БД
   * */
  DBConnectionManager dbm_;
};

TEST_F(DatabaseTablesTest, TableExists) {
  ASSERT_TRUE(dbm_.GetError() == ERROR_SUCCESS_T);
  std::vector<db_table> tables{table_book, table_translation, table_author};
  for (const auto& x : tables) {
    if (!dbm_.IsTableExists(x)) {
      dbm_.CreateTable(x);
      ASSERT_TRUE(dbm_.IsTableExists(x));
    }
  }
}

/**
 * \brief Тест на добавлени книг к таблице моделей
 * */
TEST_F(DatabaseTablesTest, InsertBooks) {
  WhereTreeConstructor<table_book> c(&adb_);
  std::string str = "string by gtest";
  /* insert */
  book divine_comedy;
  book_construct(divine_comedy, -1, lang_ita, "Divina Commedia", 1320,
                 book::f_full & ~book::f_id);
  WhereTree wt1(c);
  wt1.Init(c.And(c.Eq(BOOK_LANG, int(lang_ita)),
                 c.Eq(BOOK_TITLE, "Divina Commedia"),
                 c.Eq(BOOK_PUB_YEAR, 1320)));
  /* Удалим строку если она уже есть */
  auto st = dbm_.DeleteRows(wt1);
  int dc_id = -1;
  st = dbm_.SaveSingleRow(divine_comedy, &dc_id);
  EXPECT_GE(dc_id, 0);

  /* select */
  std::vector<book> r;
  dbm_.SelectRows(wt1, &r);

  ASSERT_TRUE(r.size() > 0);
  EXPECT_GT(r[0].id, 0);
  EXPECT_EQ(r[0].lang, lang_ita);
  EXPECT_EQ(r[0].title, "Divina Commedia");
  EXPECT_EQ(r[0].first_pub_year, 1320);

  /* delete */
  book dc_del;
  dc_del.id = r[0].id;
  WhereTree wt2(c);
  wt2.Init(c.Eq(BOOK_ID, dc_del.id));
  st = dbm_.DeleteRows(wt2);
  ASSERT_TRUE(is_status_ok(st));

  /* check delete */
  r.clear();
  st = dbm_.SelectRows(wt1, &r);
  EXPECT_TRUE(r.empty());
  ASSERT_TRUE(is_status_ok(st));

  /* add SaveNotExists */
  book quixote;
  book_construct(quixote, -1, lang_esp,
                 "El ingenioso hidalgo don Quijote de la Mancha", 1605,
                 book::f_full & ~book::f_id);
  r.push_back(quixote);
  id_container r_id;
  st = dbm_.SaveNotExistsRows(r, &r_id);
  EXPECT_TRUE(is_status_ok(st));
}
