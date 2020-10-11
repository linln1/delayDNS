# delayDNS

## 2020/9/4  v1.0 -debug
    realized the first version of delaydns
    which can read ip-domain entities from file "dnsrelay.dns" by fault
    it could intercept some request to the domain in "dnsrelay.dns" if ip is "0.0.0.0"
       we could all also use our own delaydns to replace the default dns setup by computer automatically
    to surfing the net.

## 2020/9/5  v1.1 -debug
    + realized the second version of delaydns
    + add cache function by using FIFO/LRU algorithm
    

## 2020/9/6  v1.2 -debug
    + fixed some bugs
    + rewrite the logic

## 2020/9/7  v1.3 --debug
    + change two socket structure to one socket structure
    
## 2020/9/12  v1.4 --release
    
## [Usage]
    You can test it by use command 
        ./LocalDNS [-d|-dd] [x.x.x.x] ["filepath"]
    which parameters are explained as follow:
        ~ [-d|-dd] the level of message your can get from the shell
        ~ [x.x.x.x] the ip address of the remoteDNS 
        ~ "filepath" the file path that ip-domain table exists

    You can also use it in two computers to test
    one of which should be delayDNS
    then one of it send request message to delayDNS 
    waiting for the result back from it

## [Function]
    - sheild the sensitive ip address and webpage
    - recieve and transfer ipv4, ipv6 packet to default remote Server
    - cache the recently visited ip address and website domain
    - be a middle server to provide function of IP lookup
    
## [To do list]
    - Change non-blocking socket function to multi-thread method
