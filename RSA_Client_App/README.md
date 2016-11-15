RSA Memory Server and Client
==

This is a server which serves pages upon request by the libbdvmi
RSA Application.

rsa_mem_server
--

This is the server which listens for requests consisting of a DomU's
physical and virtual memory address, and sends responses consisting of
an allocated page of memory which that DomU will use.

rsa_mem_client
--

This is an example client which sends a "dummy" request to the server
and gets the response.  This is primarily for **debugging** purposes.
