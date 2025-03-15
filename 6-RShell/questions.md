1. How does the remote client determine when a command's output is fully received from the server, and what techniques can be used to handle partial reads or ensure complete message transmission?

--> The remote client determines when a command's output is fully received from the server by looking for the EOF character (0x04). In the code, this is defined as the constant RDSH_EOF_CHAR in rshlib.h. When receiving data, the client continually reads from the socket until it encounters this special character, which signals the end of the server's response. To handle partial reads or ensure complete message transmission, the code uses a loop that keeps receiving data until the EOF character is found. The client also checks the return value of recv() to detect connection issues or closures, handling cases where io_size is negative (error) or zero (server closed connection).

2. This week's lecture on TCP explains that it is a reliable stream protocol rather than a message-oriented one. Since TCP does not preserve message boundaries, how should a networked shell protocol define and detect the beginning and end of a command sent over a TCP connection? What challenges arise if this is not handled correctly?
--> A networked shell protocol should define clear message boundaries using techniques like special delimiter characters, length prefixing, or encapsulation. In our shell implementation, we use the EOF character (0x04) as an explicit end-of-message marker. The server appends this character after sending the complete command output using the send_message_eof() function. If message boundaries aren't properly handled, several issues can arise: command outputs might blend together, partial messages could be interpreted as complete ones, the client might wait indefinitely for more data, or sensitive information could leak between different command responses.


3. Describe the general differences between stateful and stateless protocols.
--> tateful protocols maintain information about the client-server interaction across multiple messages. They remember previous exchanges and can use this context for subsequent communications. Examples include TCP and HTTP sessions with cookies. Stateless protocols, in contrast, treat each message independently without retaining information about previous interactions. 


4. Our lecture this week stated that UDP is "unreliable". If that is the case, why would we ever use it?
-->Despite being "unreliable," UDP has several important use cases that make it valuable:->

Speed: UDP has lower latency as it doesn't require connection setup or acknowledgment
Efficiency: It has less overhead without reliability mechanisms




5. What interface/abstraction is provided by the operating system to enable applications to use network communications?
--> The operating system provides the Socket API as the primary interface/abstraction for network communications. This interface includes functions like socket(), bind(), listen(),which abstract away the complex details of network protocols. It ets applications create connection endpoints, establish connections, and transfer data without dealing with the low-level details of packet formatting, routing, or protocol specifics. 


