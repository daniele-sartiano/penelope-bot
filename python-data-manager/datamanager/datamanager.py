import orjson
from cassandra.io.libevreactor import LibevConnection
from cassandra.cluster import Cluster


class DataManager:

    KEYSPACE = 'test'
    TABLE = 'data'

    def __init__(self, hosts):
        cluster = Cluster(hosts)
        cluster.connection_class = LibevConnection
        self.session = cluster.connect()
        create_key_space_query = "CREATE KEYSPACE IF NOT EXISTS {} WITH replication = {{ 'class': 'SimpleStrategy', 'replication_factor': '1'}}".format(self.KEYSPACE)
        self.session.execute(create_key_space_query)

        create_table_query = "CREATE TABLE IF NOT EXISTS {}.{} (timestamp bigint, link text, text text, ip inet, links set<text>, PRIMARY KEY (link))".format(self.KEYSPACE, self.TABLE)
        self.session.execute(create_table_query)

    @staticmethod
    def deserialize(obj):
        return orjson.loads(obj)

    def insert(self, obj):
        insert_obj_query = 'INSERT INTO {}.{} (timestamp, link, text, ip, links) VALUES (%(timestamp)s, %(link)s, %(text)s, %(ip)s, %(links)s)'.format(self.KEYSPACE, self.TABLE)
        obj['links'] = set(obj['links'])
        self.session.execute(insert_obj_query, obj)

    def get_model(self, link):
        query = 'SELECT count(*) FROM {}.{} WHERE link=%s'.format(self.KEYSPACE, self.TABLE)
        r = self.session.execute(query, [link])
        return {'timestamp': 0, 'link': link, 'text': '', 'filename': '', 'ip': '', 'links': []} if r[0].count == 0 else None

    def get_models_to_download(self, m, threshold=10):
        models = []
        for link in m['links']:
            m = self.get_model(link)
            if m is not None:
                models.append(m)
            if len(models) == threshold:
                yield orjson.dumps({'models': models})
                models = []
        if models:
            yield orjson.dumps({'models': models})

