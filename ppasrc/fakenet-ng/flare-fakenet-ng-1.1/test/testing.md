# Test Procedure (Work In Progress)

FakeNet-NG currently supported platforms as of this writing (2):
* Windows
* Linux

FakeNet-NG supported operating modes:
* SingleHost
* MultiHost (Linux only as of this writing)

FakeNet-NG supported MultiHost clients:
* Windows
* Linux

Each platform and mode is tested in each applicable test suite (below) against
all supported clients, as follows:
* Windows SingleHost
    * SingleHost Mode
    * General - Port and IP Redirection
    * General - Listeners
* Linux SingleHost
    * SingleHost Mode
    * General - Port and IP Redirection
    * General - Listeners
* Linux MultiHost (Linux client)
    * MultiHost Mode
    * General - Port and IP Redirection
    * General - Listeners
* Linux MultiHost (Windows client)
    * MultiHost Mode
    * General - Port and IP Redirection
    * General - Listeners

Any order in which the test suites are arranged will engender some jumping
around between different sections of the configuration. If you think a test is
missing, be sure to check the General Test Suite or the appropriate
mode-specific test suite before adding it.

# Pre-conditions:

Optional:
* Change VM networking to Host-Only if applicable

# Test Suites

## MultiHost Mode Test Suite

Applies to:
* Linux MultiHost

Test cases:

0. Diverter Settings
    * NetworkMode: `NetworkMode: MultiHost` (no test)
    * Comment out LinuxRedirectNonlocal e.g. `# LinuxRedirectNonlocal: *`
        * e.g. `wget -T 10 8.8.8.8` should time out
    * Enable LinuxRedirectNonlocal e.g. `LinuxRedirectNonlocal: *` - redirects foreign-bound packets to localhost
        * e.g. `wget -T 10 8.8.8.8` should retrieve page

## SingleHost Mode Test Suite

Applies to:
* Windows SingleHost
* Linux SingleHost

Test cases:

0. Diverter Settings
    * NetworkMode: SingleHost (Linux only)
    * DumpPackets - causes pcaps to be written
    * DumpPacketsFilePrefix - changes pcap name prefix
    * FixDNS
        * Windows: sets to x.x.x.254 IF it was not already set
        * Linux: no change
    * FixGateway setting
        * MultiHost mode: makes no change
        * SingleHost mode: when disabled, makes no change
        * SingleHost mode: when enabled, sets default route to a non-loopback interface IP
        * When FakeNet-NG exits: the route need not be actively removed - this is the meaning of the "fix" semantic
    * ModifyLocalDNS unconditionally sets to local IP (not 127.x.x.x)
    * LinuxFlushDNSCommand (Linux only)
    * StopDNSService
        * Windows: stops DNS service
        * Linux: no change
    * ProcessBlackList (global)
		* Set the default TCP listener to HTTPListener80 for these tests: `DefaultTCPListener: HTTPListener80`
        * Add netcat to the global process blacklist e.g. `ProcessBlackList: nc` (or `nc.exe` on Windows)
		* Use netcat to make HTTP requests, e.g. `echo GET / HTTP/1.0 | nc -v host port`
			* Arbitrary IP - observe normal operation - observe **failure**
			* Non-standard port - observe **failure**
			* Start a listener on a non-standard port and connect to it - observe normal operation with exception of connection never closing on client end (standing issue)
			* Both - observe **failure**
		* Use wget to make HTTP requests, e.g. `wget host:port`
			* Arbitrary IP - observe normal operation
			* Non-standard port - observe normal operation
			* Both - observe normal operation
    * ProcessWhiteList (global)
		* Set the default TCP listener to HTTPListener80 for these tests: `DefaultTCPListener: HTTPListener80`
        * Add netcat to the global process whitelist e.g. `ProcessWhiteList: nc` (or `nc.exe` on Windows)
		* Use netcat to make HTTP requests, e.g. `echo GET / HTTP/1.0 | nc -v host port`
			* Arbitrary IP - observe normal operation - observe **failure**
			* Non-standard port - observe **failure**
			* Start a listener on a non-standard port and connect to it - observe normal operation with exception of connection never closing on client end (standing issue)
			* Both - observe **failure**
		* Use wget to make HTTP requests, e.g. `wget host:port`
			* Arbitrary IP - observe normal operation
			* Non-standard port - observe normal operation
			* Both - observe normal operation
    * HostBlackList (global)

0. Listener Settings
    * ExecuteCmd - e.g. `echo "Process {procname} ({pid}) {src_addr}:{src_port}->{dst_addr}:{dst_port}" 1> ~whoever/flag.txt`
    * ProcessBlackList (per-listener)
    * ProcessWhiteList (per-listener)
    * HostWhiteList (per-listener)
    * HostBlackList (per-listener)

## General Test Suite

Applies to:
* Windows
* Linux MultiHost
* Linux SingleHost

Pre-conditions:
* Linux only: `LinuxRedirectNonlocal: *`

### Port and IP Redirection

Test cases:

0. TCP
    * Test FakeNet-NG host @ bound port e.g. `echo hi | nc -v 192.168.x.x 1337`
    * Test FakeNet-NG host @ unbound port e.g. `echo hi | nc -v 192.168.x.x 1338`
    * Test arbitrary host @ bound port e.g. `echo hi | nc -v 8.8.8.8 1337`
    * Test arbitrary host @ unbound port e.g. `echo hi | nc -v 8.8.8.8 1338`
    * Test named name @ bound port e.g. `echo hi | nc -v www.fireeye.com 1337`
    * Test named name @ unbound port e.g. `echo hi | nc -v www.fireeye.com 1338`
0. UDP
    * Test FakeNet-NG host / bound port e.g. `echo hi | nc -u -v 192.168.x.x 1337`
    * Test FakeNet-NG host / unbound port e.g. `echo hi | nc -u -v 192.168.x.x 1338`
    * Test arbitrary host / bound port e.g. `echo hi | nc -u -v 8.8.8.8 1337`
    * Test arbitrary host / unbound port e.g. `echo hi | nc -u -v 8.8.8.8 1338`
    * Test named host / bound port e.g. `echo hi | nc -u -v www.fireeye.com 1337`
    * Test named host / unbound port e.g. `echo hi | nc -u -v www.fireeye.com 1338`
0. ICMP
    * ping FakeNet-NG host e.g. `ping 192.168.x.x`
    * ping arbitrary host e.g. `ping 8.8.8.8`
    * ping named host e.g. `ping www.mandiant.com`
    * (Verify these are logged as well)
0. SSL decode
    * (Pre-condition: `DumpPackets: Yes`)
    * (Pre-condition: `DefaultTCPListener: HTTPListener443`)
    * Initiate an HTTPS connection with an arbitrary IP and port
    * Observe correct operation
    * Terminate the connection
    * Terminate FakeNet-NG
    * Load the resulting FakeNet-NG pcap in WireShark
    * Add SSL certificates if necessary
    * Observe decryption of SSL into app
0. Diverter
    * RedirectAllTraffic - disabling stops port redirection
        * Should work: `echo asdf | nc -v 192.168.x.x 1337` (no port or IP redirection necessary)
        * Will **fail** on Windows: `echo asdf | nc -v localhost 1337` (IP redirection is disabled)
        * Should **fail**: `echo asdf | nc -v anyname.com 1337` (IP redirection is disabled)
        * Should **fail**: `echo asdf | nc -v localhost 1339` (port redirection is disabled)
        * Should **fail**: `echo asdf | nc -v anyname.com 1339` (IP and port redirection are disabled)
    * DefaultTCPListener - `DefaultTCPListener: HTTPListener80` (`wget blah.com 9999`)
    * DefaultUDPListener - `DefaultUDPListener: DNS Server` (nslookup: `set port=12345`)
    * BlacklistPortsUDP
    * BlacklistPortsTCP

### Listeners

Test cases:

0. DNS - nslookup anyname e.g. `nslookup www.isightpartners.com`
0. HTTP - wget anyhost e.g. `wget www.example.com`
0. FTP (works with non-localhost e.g. `ftp ftp.fireeye.com` but not with localhost)
    * user
    * pass
    * ls
    * type binary
    * get FakeNet.gif
0. TFTP
0. IRC
0. POP
0. Raw
0. SMTP
