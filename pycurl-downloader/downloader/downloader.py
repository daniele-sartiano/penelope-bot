import sys
import orjson
import pycurl
import hashlib

class Downloader:

    USER_AGENT = "penelope-bot"

    def __init__(self, directory):
        self.directory = directory
        self.prefix = "{}/".format(self.directory) if self.directory else ""
        
    def download(self, msg):
        models = orjson.loads(msg)['models']

        #print(models, file=sys.stderr)

        #https://github.com/pycurl/pycurl/blob/master/examples/retriever-multi.py
        
        # Pre-allocate a list of curl objects
        m = pycurl.CurlMulti()
        m.handles = []

        for i in range(len(models)):
            c = pycurl.Curl()
            c.fp = None
            c.setopt(c.FOLLOWLOCATION, True)
            c.setopt(pycurl.MAXREDIRS, 5)
            c.setopt(pycurl.CONNECTTIMEOUT, 30)
            c.setopt(pycurl.TIMEOUT, 300)
            c.setopt(pycurl.NOSIGNAL, 1)
            c.setopt(pycurl.USERAGENT, self.USER_AGENT)
            m.handles.append(c)            

        num_processed = 0
        queue = [m for m in models]
        freelist = m.handles[:]
        models_out = []
        while num_processed < len(models):

            while queue and freelist:
                model = queue.pop()
                filename = '{}page.{}.out'.format(self.prefix, hashlib.md5(model['link'].encode()).hexdigest())
                
                c = freelist.pop()
                c.fp = open(filename, 'wb')
                c.setopt(c.URL, model['link'])
                c.setopt(c.WRITEDATA, c.fp)
                c.filename = filename
                c.url = model['link']
                c.model = model
                m.add_handle(c)

            while True:
                ret, num_handles = m.perform()
                #print(ret, num_handles, file=sys.stderr)
                if ret != pycurl.E_CALL_MULTI_PERFORM:
                    break

            #ret = m.select(1)

            while True:
                num_q, ok_list, err_list = m.info_read()
                #print(num_q, ok_list, err_list, file=sys.stderr)
                for c in ok_list:
                    c.fp.close()
                    c.fp = None
                    m.remove_handle(c)
                    model = c.model
                    model['timestamp'] = -1
                    model['ip'] = str(c.getinfo(c.PRIMARY_IP))
                    model['filename'] = c.filename
                    models_out.append(model)

                    if len(models_out) % 10 == 0:
                        yield orjson.dumps({'models': models_out})
                        models_out = []

                    #print("Success:", c.filename, c.url, c.getinfo(pycurl.EFFECTIVE_URL), file=sys.stderr)
                    #print("Success:", c.getinfo(pycurl.TOTAL_TIME), c.getinfo(pycurl.EFFECTIVE_URL), file=sys.stderr)
                    freelist.append(c)
                for c, errno, errmsg in err_list:
                    c.fp.close()
                    c.fp = None
                    m.remove_handle(c)
                    print("Failed: ", c.filename, c.url, errno, errmsg, file=sys.stderr)
                    freelist.append(c)
                num_processed = num_processed + len(ok_list) + len(err_list)
                if num_q == 0:
                    break

        # Cleanup
        for c in m.handles:
            if c.fp is not None:
                c.fp.close()
                c.fp = None
            c.close()
        m.close()
        #print(models_out, file=sys.stderr)
        if models_out:
            yield orjson.dumps({'models': models_out})
