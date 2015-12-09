# rocksdb
php rocksdb 扩展

调用方法：
    $dbPath = "/tmp/test";

    $rocksdb = new Rocksdb($dbPath);

    $rocksdb->set("key", "value");

    $rocksdb->get("key");

    $rocksdb->close();
