Notes on Sockets
===

- UDP packets may not arrive at the destiny

- Data must be in big-endian order for transmission.

- 10.x.x.x and 192.168.x.x are reserved addresses for private networks.

- NAT (network address translation) works by translating a private IP address into another IP address which is visible to the outside world.

	- This provides control and protection over the local LAN.

	- Translation is performed by directly modifying the address on the header of the packet sent by the private machine.

	- Equipments that perform NAT: firewalls, routers...

- Browsers ask the OS for an available port, and this port is used as source ports for HTTP requests (which usually have destination port 80).

- (source IP, source port, dest IP, dest port) forms a socket.

- The router has an external address, though which external hosts can send requests to it.

	- Port forwarding means pass requests from the router to the designated host.

- In a private network, 2 hosts can be trying to communicate externally using the same source port.

	- The response packet from the websites don't contain the IP of the host machines, so how does the router know which machine should receive the packet?

	- The router reads each packet's source port and source address;

	- Then it allocates an available port, and changes the original source port with the new one;

	- The source address is also changed to the router's external address;

	- Finally, the router stores internally a mapping from the new address/port to the old address/port, which will be used to revert the changes when the reply packet arrives.
