import random
import string
import hashlib
import logging

import requests
from requests import Request
import simplejson as json
from tests.utils import BaseTestCase


class TestMeta2Functional(BaseTestCase):

    def url_srvtype(self, t):
        url = self.conf['proxyd_uri']
        return '/'.join((url, 'v2.0/cs', self.ns, t))

    def url_ref(self, ref):
        url = self.conf['proxyd_uri']
        return '/'.join((url, 'v2.0/dir', self.ns, self.account, ref))

    def url_container(self, ref):
        url = self.conf['proxyd_uri']
        return '/'.join((url, 'v2.0/m2', self.ns, self.account, ref))

    def setUp(self):
        super(TestMeta2Functional, self).setUp()
        self.ns = self.conf['namespace']
        self.proxyd = self.conf['proxyd_uri'] + "/v2.0"
        self.account = self.conf['account']
        self.chars = (string.ascii_lowercase + string.ascii_uppercase +
                      string.digits)

        self.proxyd_dir = self.proxyd + "/dir/" + self.ns + '/' + self.account
        self.proxyd_m2 = self.proxyd + "/m2/" + self.ns + '/' + self.account

        self.h = hashlib.new('md5')

        self.id_generator = lambda n: ''.join(
            random.choice(self.chars) for _ in range(n))
        self.session = requests.session()

        self.list_paths = list()

        self.idRef_rand = self.id_generator(6)
        self.idRef_rand2 = self.id_generator(6)
        self.prop = self.id_generator(8)
        self.path_rand = self.id_generator(10)
        self.path_rand2 = self.id_generator(10)
        self.prop_rand = self.id_generator(12)
        self.prop_rand2 = self.id_generator(12)
        self.string_rand = self.id_generator(20)
        self.string_rand2 = self.id_generator(20)

        self.h.update(self.string_rand)
        self.hash_rand = self.h.hexdigest()
        self.h.update(self.string_rand2)
        self.hash_rand2 = self.h.hexdigest()

        self.addr_Ref = self.url_ref(self.idRef_rand)
        self.addr_Ref_type = self.url_ref(self.idRef_rand) + "/meta2"
        self.addr_m2_ref = self.proxyd_m2 + "/" + self.idRef_rand
        self.addr_m2_ref_action = self.addr_m2_ref + "/action"
        self.addr_m2_ref_path = self.addr_m2_ref + "/" + self.path_rand
        self.addr_m2_ref_path2 = self.addr_m2_ref + "/" + self.path_rand2
        self.addr_Ref_type_action = self.addr_Ref_type + "/action"
        self.addr_m2_ref_path_action = self.addr_m2_ref_path + "/action"

        self.direct_path = "NS/" + self.account + "/" + self.idRef_rand + \
                           "/" + self.path_rand
        self.path_paste = "NS/" + self.account + "/" + self.idRef_rand + \
                          "/" + self.path_rand2
        self.path_paste_wrong = "NS/" + self.account + "/" + \
                                self.idRef_rand2 + "/" + self.path_rand

        self.addr_alone_ref = self.proxyd_dir + "/" + self.idRef_rand2
        self.addr_alone_ref_type_action = self.addr_alone_ref + "meta2/action"
        self.addr_m2_alone_ref = self.proxyd_m2 + "/" + self.idRef_rand2
        self.addr_m2_alone_ref_path = (self.addr_m2_alone_ref + "/" +
                                       self.path_rand)
        self.addr_m2_alone_ref_action = self.addr_m2_alone_ref + "/action"

        self.invalid_addr_m2 = self.proxyd_m2 + "/error"
        self.invalid_addr_m2_action = self.invalid_addr_m2 + "/action"

        self.bean_void = {'size': ''}
        self.bean = self.bean_void

        resp = self.session.put(self.addr_Ref)
        self.assertEqual(201, resp.status_code)

        resp = self.session.put(self.addr_alone_ref)
        self.assertEqual(201, resp.status_code)

        action = {"action": "Link", "args": None}
        resp = self.session.post(self.addr_Ref_type_action, json.dumps(action))
        self.assertEqual(200, resp.status_code)

        logging.debug("+++")

    def tearDown(self):
        super(TestMeta2Functional, self).tearDown()

        for a in [self.addr_m2_ref_path, self.addr_m2_ref_path2,
                  self.addr_m2_ref, self.addr_Ref_type,
                  self.addr_Ref]:
            try:
                self.session.delete(a)
            except Exception:
                pass

    def prepare_bean(self, i):

        if i == 1:
            action = {"action": "Beans", "args": self.bean_void}
            resp = self.session.post(self.addr_m2_ref_path_action,
                                     json.dumps(action))
            self.assertEqual(resp.status_code, 200)
            beans = resp.json()
            self.assertIsInstance(beans, list)
            self.assertGreaterEqual(len(beans), 1)
            self.bean = beans[0]
            self.bean["id"] = self.bean["url"]
            self.bean["hash"] = self.hash_rand
            self.bean["size"] = 40
            self.bean["type"] = "chunk"
            self.prepared_bean = self.bean
        if i == 2:
            self.bean2 = \
                self.session.post(self.addr_m2_ref_path_action, json.dumps(
                    {"action": "Beans", "args": self.bean_void}
                )).json()[0]
            self.bean2["id"] = self.bean2["url"]
            self.bean2["hash"] = self.hash_rand2
            self.bean2["size"] = 80
            self.bean2["type"] = "chunk"

    def post_bean(self):

        self.session.put(self.addr_m2_ref_path,
                         json.dumps([self.prepared_bean]),
                         headers={
                             'x-oio-content-meta-hash': self.hash_rand,
                             'x-oio-content-meta-length': 40})

    def prepare_bean_list(self, i):

        j = 1

        while j <= i:
            self.path_rand = self.id_generator(10)
            self.string_rand = self.id_generator(20)

            self.h.update(self.string_rand)
            self.hash_rand = self.h.hexdigest()

            self.addr_m2_ref_path = self.addr_m2_ref + "/" + self.path_rand
            self.addr_m2_ref_path_action = self.addr_m2_ref_path + "/action"

            self.prepared_bean = \
                self.session.post(self.addr_m2_ref_path_action, json.dumps(
                    {"action": "Beans", "args": self.bean_void}
                )).json()[0]
            self.prepared_bean["id"] = self.prepared_bean["url"]
            self.prepared_bean["hash"] = self.hash_rand
            self.prepared_bean["size"] = 40
            self.prepared_bean["type"] = "chunk"

            self.post_bean()

            self.list_paths.append(self.addr_m2_ref_path)
            j += 1

    def prepare_bean_listing(self):

        self.prepared_bean = \
            self.session.post(self.addr_m2_ref_path_action, json.dumps(
                {"action": "Beans", "args": self.bean_void}
            )).json()[0]
        self.prepared_bean["hash"] = self.hash_rand
        self.prepared_bean["size"] = 40
        self.prepared_bean["type"] = "chunk"

        for num1 in xrange(4):
            for num2 in xrange(13):
                self.addr_m2_ref_path = self.addr_m2_ref + '/%d-%02d' % (
                    num1, num2)
                self.addr_m2_ref_path_action = (self.addr_m2_ref_path +
                                                "/action")
                self.post_bean()
                self.list_paths.append(self.addr_m2_ref_path)

        for num in xrange(13):
            self.addr_m2_ref_path = self.addr_m2_ref + '/2-05-%02d' % num
            self.addr_m2_ref_path_action = self.addr_m2_ref_path + "/action"
            self.post_bean()
            self.list_paths.append(self.addr_m2_ref_path)

        for num in xrange(13):
            self.addr_m2_ref_path = self.addr_m2_ref + '/3-%02d-05' % num
            self.addr_m2_ref_path_action = self.addr_m2_ref_path + "/action"
            self.post_bean()
            self.list_paths.append(self.addr_m2_ref_path)

    def delete_bean_list(self):

        for a in self.list_paths:
            try:
                self.session.delete(a)
            except Exception:
                pass
        del self.list_paths

    def prepare_content(self):

        self.session.put(self.addr_m2_ref)
        self.prepare_bean(1)
        self.post_bean()

    def prepare_properties(self):

        self.prepare_content()
        self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "SetProperties",
             "args": {"prop1": self.prop_rand, "prop2": self.prop_rand2}}
        ))

    # Containers tests

    def test_containers_put(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.head(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

    def test_container_head(self):
        self.session.put(self.addr_m2_ref)
        resp = self.session.head(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

    def test_container_head_ref_no_link(self):
        resp = self.session.head(self.addr_m2_alone_ref)
        self.assertEqual(resp.status_code, 404)

    def test_container_head_ref_link(self):
        resp = self.session.head(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

    def test_container_get(self):
        self.prepare_content()
        resp = self.session.get(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 200)
        self.assertEqual(type(resp.json()), dict)

    def test_containers_get_marker(self):

        self.session.put(self.addr_m2_ref)
        self.prepare_bean_list(5)

        before_mark = [bean["name"] for bean in
                       self.session.get(self.addr_m2_ref,
                                        params={'marker_end': 'W'}).json()[
                           "objects"]]
        after_mark = [bean["name"] for bean in
                      self.session.get(self.addr_m2_ref,
                                       params={'marker': 'W'}).json()[
                          "objects"]]

        for name in before_mark:
            self.assertTrue(name <= 'W')
        for name in after_mark:
            self.assertTrue(name > 'W')

        self.delete_bean_list()

    def test_containers_get_prefix(self):

        self.session.put(self.addr_m2_ref)
        self.prepare_bean_list(2)

        marker = self.session.get(self.addr_m2_ref).json()['objects'][0]
        marker = marker['name'][0:3]

        prefix_mark = [bean["name"] for bean in
                       self.session.get(self.addr_m2_ref,
                                        params={'prefix': marker}).json()[
                           "objects"]]

        for name in prefix_mark:
            self.assertTrue(name[0:3] == marker)

        self.delete_bean_list()

    def test_containers_get_delimiter(self):

        self.session.put(self.addr_m2_ref)
        self.prepare_bean_list(5)

        names = [bean['name'] for bean in
                 self.session.get(self.addr_m2_ref).json()['objects']]

        marker = self.session.get(self.addr_m2_ref).json()['objects'][0]
        marker = marker['name'][2:3]

        delimit_mark = [bean["name"] for bean in
                        self.session.get(self.addr_m2_ref,
                                         params={'delimiter': marker}).json()[
                            "objects"]]

        for bean in delimit_mark:
            self.assertTrue(marker not in bean)
        for bean in names:
            if bean not in delimit_mark:
                self.assertTrue(marker in bean)

        self.delete_bean_list()

    def test_containers_get_max(self):

        self.session.put(self.addr_m2_ref)
        self.prepare_bean_list(5)

        max_mark = [bean["name"] for bean in
                    self.session.get(self.addr_m2_ref,
                                     params={'max': 3}).json()[
                        "objects"]]

        self.assertEqual(len(max_mark), 3)

        self.delete_bean_list()

    def test_container_get_ref_link(self):
        # FIXME the reference exists, requesting the container currently
        # autocreates it.
        resp = self.session.get(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 404)

    def test_container_get_listing(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean_listing()

        resp = self.session.get(self.addr_m2_ref, params={'max': 10})
        self.assertEqual(resp.status_code, 200)
        listing = resp.json()["objects"]

        self.assertEqual(len(listing), 10)
        self.assertEqual(listing[0]['name'], '0-00')
        self.assertEqual(listing[-1]['name'], '0-09')

        listing = self.session.get(self.addr_m2_ref,
                                   params={'max': 10,
                                           'marker_end': '0-05'}).json()[
            "objects"]

        self.assertEqual(len(listing), 5)
        self.assertEqual(listing[0]['name'], '0-00')
        self.assertEqual(listing[-1]['name'], '0-04')

        listing = self.session.get(self.addr_m2_ref,
                                   params={'max': 10,
                                           'marker': '0-09'}).json()[
            "objects"]

        self.assertEqual(len(listing), 10)
        self.assertEqual(listing[0]['name'], '0-10')
        self.assertEqual(listing[-1]['name'], '1-06')

        listing = self.session.get(self.addr_m2_ref,
                                   params={'max': 6, 'marker': '1-09'}).json()[
            "objects"]

        self.assertEqual(len(listing), 6)
        self.assertEqual(listing[0]['name'], '1-10')
        self.assertEqual(listing[-1]['name'], '2-02')

        listing = self.session.get(self.addr_m2_ref,
                                   params={'max': 2, 'prefix':
                                           '0-1'}).json()["objects"]

        self.assertEqual(len(listing), 2)
        self.assertEqual(listing[0]['name'], '0-10')
        self.assertEqual(listing[-1]['name'], '0-11')

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 2, 'delimiter': '-',
                                     'prefix': '0-1'}).json()[
                "objects"]

        self.assertEqual(len(listing), 2)
        self.assertEqual(listing[0]['name'], '0-10')
        self.assertEqual(listing[-1]['name'], '0-11')

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 5, 'delimiter': '-',
                                     'prefix': '0-'}).json()[
                "objects"]

        self.assertEqual(len(listing), 5)
        self.assertEqual(listing[0]['name'], '0-00')
        self.assertEqual(listing[-1]['name'], '0-04')

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 10, 'delimiter': '-'}).json()[
                "prefixes"]

        self.assertEqual(len(listing), 4)
        self.assertEqual(listing, ['0-', '1-', '2-', '3-'])

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 10, 'marker': '2-',
                                     'delimiter': '-'}).json()["prefixes"]

        self.assertEqual(len(listing), 2)
        self.assertEqual(listing, ['2-', '3-'])

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 10, 'prefix': '2',
                                     'delimiter': '-'}).json()["prefixes"]

        self.assertEqual(len(listing), 1)
        self.assertEqual(listing, ['2-'])

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 6, 'prefix': '2-',
                                     'marker': '2-04',
                                     'delimiter': '-'}).json()

        self.assertEqual(len(listing["objects"]), 5)
        self.assertEqual(listing["objects"][0]['name'], '2-05')
        self.assertEqual(listing["objects"][1]['name'], '2-06')
        self.assertEqual(listing["objects"][-1]['name'], '2-09')
        self.assertEqual(listing["prefixes"], ['2-05-'])

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 10, 'prefix': '3-',
                                     'marker': '3-05',
                                     'delimiter': '-'}).json()

        self.assertEqual(len(listing["objects"]), 5)
        self.assertEqual(len(listing["prefixes"]), 5)
        self.assertEqual([obj["name"] for obj in listing["objects"]],
                         ['3-06', '3-07', '3-08', '3-09', '3-10'])
        self.assertEqual(listing["prefixes"],
                         ['3-05-', '3-06-', '3-07-', '3-08-', '3-09-'])

        listing = \
            self.session.get(self.addr_m2_ref,
                             params={'max': 10, 'marker': '3-05'}).json()[
                "objects"]

        self.assertEqual(len(listing), 10)
        self.assertEqual([obj["name"] for obj in listing],
                         ['3-05-05', '3-06', '3-06-05', '3-07', '3-07-05',
                          '3-08', '3-08-05', '3-09', '3-09-05', '3-10'])

        #  test_get_listing6

        self.delete_bean_list()

    # Failed get_list tests

    def test_container_get_listing6(self):

        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean_listing()

        params = {'max': 10, 'prefix': '3-05-', 'delimiter': "-"}
        resp = self.session.get(self.addr_m2_ref, params=params)
        self.assertEqual(resp.status_code, 200)
        listing = resp.json()

        self.assertEqual(len(listing["objects"]), 1)
        self.assertEqual(listing["objects"][0]["name"], '3-05-05')
        self.assertEqual(listing["prefixes"], ['3-05-'])

        self.delete_bean_list()

    # End of failed get_list tests

    def test_container_delete(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        resp = self.session.delete(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        # FIXME the container will be recreated by the HEAD request
        resp = self.session.head(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 404)

    def test_container_delete_ref_no_link(self):
        resp = self.session.delete(self.addr_m2_alone_ref)
        self.assertEqual(resp.status_code, 404)
        resp = self.session.delete(self.addr_m2_alone_ref)
        self.assertEqual(resp.status_code, 404)

    def test_container_delete_ref_link(self):
        resp = self.session.delete(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        # FIXME the container will be recreated by the HEAD request
        resp = self.session.delete(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 404)

    # Containers Actions tests

    def test_containers_actions_touch(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        action = {'action': 'Touch', 'args': 'chunk'}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 500)

    # def test_containers_actions_dedup(self):  # ignored

    # def test_containers_actions_purge(self):  # ignored

    def test_containers_actions_setStoragePolicy(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "SetStoragePolicy", "args": "TWOCOPIES"}
        ))
        self.assertEqual(resp.status_code, 200)

        h = {'x-oio-content-meta-hash': self.hash_rand,
             'x-oio-content-meta-length': 40}
        resp = self.session.put(self.addr_m2_ref_path,
                                json.dumps([self.bean]), headers=h)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref)
        objects = resp.json()["objects"]
        first = objects[0]
        self.assertEqual(first["policy"], "TWOCOPIES")

    def test_containers_actions_setStoragePolicy_ref_no_link(self):
        action = {"action": "SetStoragePolicy", "args": "TWOCOPIES"}
        resp = self.session.post(self.addr_m2_alone_ref_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_setStoragePolicy_ref_link(self):
        action = {"action": "SetStoragePolicy", "args": "TWOCOPIES"}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_setStoragePolicy_wrong(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)
        action = {"action": "SetStoragePolicy", "args": "error"}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 403)

        h = {'x-oio-content-meta-hash': self.hash_rand,
             'x-oio-content-meta-length': 40}
        resp = self.session.put(self.addr_m2_ref_path,
                                json.dumps([self.bean]), headers=h)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 200)
        objects = resp.json()
        first = objects["objects"][0]
        self.assertEqual(first["policy"], "none")

    def test_containers_actions_getProperties(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        action = {"action": "GetProperties", "args": None}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 200)
        self.assertEqual(type(resp.json()), dict)

    def test_containers_actions_getProperties_ref_no_link(self):
        action = {"action": "GetProperties", "args": None}
        resp = self.session.post(self.addr_m2_alone_ref_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_getProperties_ref_link(self):
        action = {"action": "GetProperties", "args": None}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 200)

    def test_containers_actions_setProperties(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "SetProperties", "args": {"sys.user.name": self.prop}}
        ))
        self.assertEqual(resp.status_code, 200)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "GetProperties", "args": None}
        )).json()["sys.user.name"]
        self.assertEqual(resp, self.prop)

    def test_containers_actions_setProperties_wrong(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "SetProperties", "args": {"error": self.prop}}
        ))
        self.assertEqual(resp.status_code, 400)

    def test_containers_actions_setProperties_ref_no_link(self):
        resp = self.session.post(self.addr_m2_alone_ref_action, json.dumps(
            {"action": "SetProperties", "args": {"sys.user.name": self.prop}}
        ))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_setProperties_ref_link(self):
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "SetProperties", "args": {"sys.user.name": self.prop}}
        ))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_DelProperties(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "DelProperties", "args": ["sys.user.name"]}
        ))
        self.assertEqual(resp.status_code, 200)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "GetProperties", "args": None}
        )).json()
        self.assertFalse("sys.user.name" in resp.keys())

    def test_containers_actions_DelProperties_no_link(self):
        resp = self.session.post(self.addr_m2_alone_ref_action, json.dumps(
            {"action": "DelProperties", "args": ["sys.user.name"]}
        ))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_DelProperties_link(self):
        action = {"action": "DelProperties", "args": ["sys.user.name"]}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_containers_actions_rawInsert(self):  # to be improved
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)

        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "RawInsert", "args": [self.bean]}
        ))
        self.assertEqual(resp.status_code, 204)

    def test_containers_actions_rawDelete(self):  # to be improved
        self.prepare_content()
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(
            {"action": "RawDelete", "args": [self.bean]}
        ))
        self.assertEqual(resp.status_code, 204)

    def test_containers_actions_rawUpdate(self):
        self.prepare_content()
        self.prepare_bean(2)

        action = {"action": "RawUpdate",
                  "args": {"new": [self.bean2], "old": [self.bean]}}
        resp = self.session.post(self.addr_m2_ref_action, json.dumps(action))
        self.assertTrue(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 200)
        beans = resp.json()
        self.assertGreaterEqual(len(beans), 1)
        first = beans[0]
        self.assertEqual(first["url"], self.bean2["url"])
        self.assertEqual(first["hash"], self.bean2["hash"].upper())

    def test_containers_actions_rawUpdate_ref_link(self):
        self.prepare_content()
        action = {"action": "Link", "args": None}
        resp = self.session.post(self.addr_alone_ref_type_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 200)

        action = {"action": "RawUpdate",
                  "args": {"new": [self.bean], "old": [self.bean]}}
        resp = self.session.post(self.addr_m2_alone_ref_action,
                                 json.dumps(action))
        self.assertTrue(resp.status_code, 404)

    # Content tests

    def test_content_head(self):
        self.prepare_content()
        resp = self.session.head(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 204)

    def test_content_head_void_container(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        resp = self.session.head(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 404)

    def test_contents_get(self):
        self.prepare_content()
        resp = self.session.get(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 200)
        self.assertEqual(resp.json()[0]["hash"], self.hash_rand.upper())

    def test_contents_get_void_container(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        resp = self.session.get(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 404)

    def test_contents_put(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)

        headers = {'x-oio-content-meta-hash': self.hash_rand,
                   'x-oio-content-meta-length': 40}
        resp = self.session.put(self.addr_m2_ref_path,
                                json.dumps([self.bean]), headers=headers)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 200)
        objects = resp.json()["objects"]
        self.assertGreaterEqual(len(objects), 1)
        first = objects[0]
        self.assertEqual(first['hash'], self.hash_rand.upper())

    def test_contents_put_ref_link(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)
        action = {"action": "Link", "args": None}
        resp = self.session.post(self.addr_alone_ref_type_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 200)

        headers = {'x-oio-content-meta-hash': self.hash_rand,
                   'x-oio-content-meta-length': 40}
        resp = self.session.put(self.addr_m2_alone_ref_path,
                                json.dumps([self.bean]), headers=headers)
        self.assertEqual(resp.status_code, 404)

    def test_contents_put_invalid_headers(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        self.prepare_bean(1)

        headers = {'x-oio-content-meta-hash': 'error',
                   'x-oio-content-meta-length': 40}
        resp = self.session.put(self.addr_m2_ref_path,
                                json.dumps([self.bean]), headers=headers)
        self.assertEqual(resp.status_code, 400)

    def test_contents_delete(self):
        self.prepare_content()

        resp = self.session.delete(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 200)

        resp = self.session.get(self.addr_m2_ref).json()['objects']
        self.assertEqual(resp, [])

    def test_contents_delete_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.delete(self.addr_m2_ref_path)
        self.assertEqual(resp.status_code, 404)

    def test_contents_copy(self):
        self.prepare_content()
        req = Request('COPY', self.addr_m2_ref_path,
                      headers={'Destination': self.path_paste})
        resp = self.session.send(req.prepare())
        self.assertEqual(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref_path).json()
        resp2 = self.session.get(self.addr_m2_ref_path2).json()
        self.assertEqual(resp, resp2)

    def test_contents_copy_no_aim(self):
        self.prepare_content()
        req = Request('COPY', self.addr_m2_ref_path,
                      headers={'Destination': self.path_paste_wrong})
        resp = self.session.send(req.prepare())
        self.assertTrue(resp.status_code, 400)

    def test_contents_copy_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)
        req = Request('COPY', self.addr_m2_ref_path,
                      headers={'Destination': self.path_paste_wrong})
        resp = self.session.send(req.prepare())
        self.assertTrue(resp.status_code, 400)

    # Content actions tests

    def test_contents_actions_beans(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "Beans", "args": self.bean_void}
        ))
        self.assertEqual(resp.status_code, 200)
        for label in ["url", "pos", "size", "hash"]:
            self.assertTrue(label in resp.json()[0].keys())

    def test_contents_actions_beans_wrong_arg(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "Beans", "args": {'error': 'error'}}
        ))
        self.assertEqual(resp.status_code, 400)

    def test_contents_actions_beans_ref_no_link(self):
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "Beans", "args": self.bean_void}
        ))
        self.assertEqual(resp.status_code, 404)

    def test_contents_actions_Spare(self):  # to be improved
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        action = {"action": "Spare",
                  "args": {"size": 40, "notin": {}, "broken": {}}}
        resp = self.session.post(self.addr_m2_ref_path_action,
                                 json.dumps(action))
        raisedException = False
        try:
            print resp.json()
        except Exception:
            raisedException = True
            pass
        self.assertFalse(raisedException)

    def test_contents_actions_Spare_Wrong(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "Spare",
             "args": {"size": 40, "notin": {}}}
        ))
        self.assertEqual(resp.status_code, 400)

    def test_contents_actions_Spare_ref_no_link(self):
        action = {"action": "Spare",
                  "args": {"size": 40, "notin": {}, "broken": {}}}
        resp = self.session.post(self.addr_m2_ref_path_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_contents_actions_touch(self):
        self.prepare_content()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "Touch", "args": self.direct_path}
        ))
        self.assertTrue(resp.status_code == 204)

    def test_contents_actions_setStoragePolicy(self):
        self.prepare_content()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "SetStoragePolicy", "args": 'TWOCOPIES'}
        ))
        self.assertEqual(resp.status_code, 204)

        resp = self.session.get(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 200)
        objects = resp.json()["objects"]
        self.assertGreaterEqual(len(objects), 1)
        first = objects[0]
        self.assertEqual(first['policy'], 'TWOCOPIES')

    def test_contents_actions_setStoragePolicy_wrong(self):
        self.prepare_content()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "SetStoragePolicy", "args": 'error'}
        ))
        self.assertEqual(resp.status_code, 400)

    def test_contents_actions_setStoragePolicy_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "SetStoragePolicy", "args": 'TWOCOPIES'}
        ))
        self.assertEqual(resp.status_code, 403)

    def test_contents_actions_getProperties(self):
        self.prepare_properties()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "GetProperties", "args": None}
        ))
        self.assertEqual(resp.status_code, 200)
        props = resp.json()
        self.assertEqual(props["prop1"], self.prop_rand)
        self.assertEqual(props["prop2"], self.prop_rand2)

    def test_contents_actions_getProperties_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "GetProperties", "args": None}
        ))
        self.assertEqual(resp.status_code, 404)

    def test_contents_actions_setProperties(self):
        self.prepare_content()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "SetProperties",
             "args": {"prop1": self.prop_rand, "prop2": self.prop_rand2}}
        ))
        self.assertEqual(resp.status_code, 200)

        action = {"action": "GetProperties", "args": None}
        resp = self.session.post(self.addr_m2_ref_path_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 200)
        props = resp.json()
        self.assertEqual(props["prop1"], self.prop_rand)
        self.assertEqual(props["prop2"], self.prop_rand2)

    def test_contents_actions_setProperties_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        action = {"action": "SetProperties",
                  "args": {"prop1": self.prop_rand,
                           "prop2": self.prop_rand2}}
        resp = self.session.post(self.addr_m2_ref_path_action,
                                 json.dumps(action))
        self.assertEqual(resp.status_code, 404)

    def test_contents_actions_delProperties(self):

        self.prepare_properties()
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "DelProperties", "args": ['prop1']}
        ))

        self.assertEqual(resp.status_code, 204)
        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "GetProperties", "args": None}
        )).json()
        self.assertFalse('prop1' in resp.keys())
        self.assertTrue('prop2' in resp.keys())

    def test_contents_actions_delProperties_no_content(self):
        resp = self.session.put(self.addr_m2_ref)
        self.assertEqual(resp.status_code, 204)

        resp = self.session.post(self.addr_m2_ref_path_action, json.dumps(
            {"action": "DelProperties", "args": ['prop1']}
        ))
        self.assertEqual(resp.status_code, 403)
