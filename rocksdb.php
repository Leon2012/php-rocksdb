<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('rocksdb')) {
	dl('rocksdb.' . PHP_SHLIB_SUFFIX);
}

$dbPath = "/tmp/rocksdb_db";
$rocksdb = new Rocksdb($dbPath);
if (!$rocksdb) {
	die("create rocksdb error!!!");
}
echo $rocksdb->get("noexists");

/*
//$rocksdb->set("key1", "hello world");
echo $rocksdb->get("key1");

//$rocksdb->set("key", "value");
$value = $rocksdb->get("key");

echo $value."\n";

$rocksdb->set("k2", "value2");
echo $rocksdb->get("k2");

echo $rocksdb->del("k2");

echo $rocksdb->get("k2");
 */

$rocksdb->close();

?>
