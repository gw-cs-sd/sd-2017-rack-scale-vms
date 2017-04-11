RSA Memory Server and Client
==

rsa_mem_server
--

Memory Server which listens for paging requests by the target application.  The server maintains a hash table of page addresses and their contents.  It responds to the RSApi functions `rsa_download_more_ram` and `rsa_upload_some_ram`.

rsa_test_client
--

Example client used to test Memory Server
