<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('rocksdb')) {
	dl('rocksdb.' . PHP_SHLIB_SUFFIX);
}

$dbPath = "/tmp/rocksdb_db";
$rocksdb = new Rocksdb($dbPath);
if (is_null($rocksdb)) {
	die("create rocksdb error!!!");
}

//$rocksdb->set("key1", "hello world");
echo $rocksdb->get("key1");

//$rocksdb->set("key", "value");
$value = $rocksdb->get("key");

echo $value."\n";

$rocksdb->close();

?>
