# delayDNS

## 2020/9/4
    realized the first version of delaydns
    which can read ip-domain entities from file "dnsrelay.dns"
    it could intecept some request to the domain in "dnsrelay.dns" if ip is "0.0.0.0"

    we could all also use our own delaydns to replace the default dns setup by computer automatically
    to surfing the net.

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

    [to do list]
        - add cache function
        - use fifo/LRU/LFU algorithms to management cache data struct
