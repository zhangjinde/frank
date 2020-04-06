#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <frk_dict.h>
#include <frk_slab.h>


const size_t size = 1024 * 10240;


void*
slab_malloc(int64_t size, void* data)
{
    frk_slab_t *s = data;
    return frk_slab_malloc(s, size, 1);
}


void
slab_free(void* ptr, void* data)
{
    frk_slab_t *s = data;
    return frk_slab_free(s, ptr);
}

void
slab_test()
{
    int i;
    void* pool = malloc(size);
    frk_slab_t *s = frk_new_slab(pool, size);

    // basic use
    int64_t *p = frk_slab_malloc(s, 100, 1);
    p[0] = 1;
    frk_slab_free(s, p);

    // test malloc free
    for (int i = 0; i < 5000; i += 23) {
        void *p = frk_slab_malloc(s, i, 1);
        // printf("malloc %d %p\n", i, p);
        frk_slab_free(s, p);
    }

    // test full
    for (int i = 0; i < 10241; i++) {
        void *p = frk_slab_malloc(s, 1024, 0);
        printf("malloc %p\n", p);
    }

    free(s);
}


void
dict_test()
{
    void* pool = malloc(size);
    frk_slab_t *s = frk_new_slab(pool, size);


    frk_dict_t* root = frk_new_dict(slab_malloc, slab_free, s);

    frk_insert_int(root, "aaa", 3, 1);
    frk_insert_int(root, "aaa", 3, 2);
    frk_insert_string(root, "bbb", 3, "xyz", 4);
    frk_insert_string(root, "bbb", 3, "xyzzzz", 4);
    frk_dict_t* sub = frk_insert_dict(root, "sub", 3);
    frk_insert_int(sub, "aaa", 3, 11);
    frk_insert_int(sub, "aaa", 3, 22);
    frk_insert_string(sub, "bbb", 3, "xyz", 4);
    frk_dict_t* subsub = frk_insert_dict(sub, "subsub", 6);
    frk_insert_int(subsub, "aaa", 3, 111);
    frk_insert_int(subsub, "aaa", 3, 222);
    frk_insert_string(subsub, "bbb", 3, "xyz", 4);

    printf("%ld\n", *frk_get_int(root, "aaa", 3));
    printf("%s\n", frk_get_str(root, "bbb", 3)->data);
    printf("%ld\n", *frk_get_int(sub, "aaa", 3));
    printf("%s\n", frk_get_str(sub, "bbb", 3)->data);
    printf("%ld\n", *frk_get_int(subsub, "aaa", 3));
    printf("%s\n", frk_get_str(subsub, "bbb", 3)->data);

    frk_remove(root, "aaa", 3);
    frk_remove(root, "bbb", 3);
    frk_remove(root, "sub", 3);

    int i = 0;
    char key[1];
    for (; i < 200; i++) {
        key[0] = (char)i;
        frk_insert_int(root, key, 1, i);
    }
    printf("%p\n", frk_get_int(root, "0", 1));
    for (i = 0; i < 200; i++) {
        key[0] = (char)i;
        frk_remove(root, key, 1);
    }
    printf("%p\n", frk_get_int(root, "0", 1));
}


void app_test()
{
    void* pool = malloc(size);
    frk_slab_t *s = frk_new_slab(pool, size);

    frk_dict_t* sites = frk_new_dict(slab_malloc, slab_free, s);

    frk_dict_t* a = frk_insert_dict(sites, "a.com", strlen("a.com"));

    frk_dict_t* wild = frk_insert_dict(a, ".a.com", strlen(".a.com"));
    frk_insert_int(wild, "timeout", strlen("timeout"), 10);
    frk_insert_int(wild, "buffer", strlen("buffer"), 4096);
    frk_insert_string(wild, "upstream", strlen("upstream"), "127.0.0.1:5000", strlen("127.0.0.1:5000"));

    frk_dict_t* www = frk_insert_dict(a, "www.a.com", strlen("www.a.com"));
    frk_insert_int(www, "timeout", strlen("timeout"), 5);
    frk_insert_int(www, "buffer", strlen("buffer"), 1024);
    frk_insert_string(www, "upstream", strlen("upstream"), "192.168.0.3:5000", strlen("192.168.0.3:5000"));

    frk_dict_t* news = frk_insert_dict(a, "news.a.com", strlen("www.a.com"));
    frk_insert_int(news, "timeout", strlen("timeout"), 5);
    frk_insert_string(news, "upstream", strlen("upstream"), "192.168.0.100:5000", strlen("192.168.0.100:5000"));
    frk_dict_t* black = frk_insert_dict(news, "black_ip", strlen("black_ip"));
    frk_insert_int(black, "222.222.222.1", strlen("222.222.222.1"), 1);
    frk_insert_int(black, "222.222.222.2", strlen("222.222.222.2"), 1);
    frk_insert_int(black, "222.222.222.3", strlen("222.222.222.3"), 1);
    frk_insert_int(black, "222.222.222.4", strlen("222.222.222.4"), 1);

    frk_dict_t* b = frk_insert_dict(sites, "b.com", strlen("b.com"));
    frk_dict_t* wild_b = frk_insert_dict(b, ".b.com", strlen(".bcom"));
    frk_insert_int(wild_b, "timeout", strlen("timeout"), 100);
    frk_insert_int(wild_b, "buffer", strlen("buffer"), 40960);

    frk_dict_t* c = frk_insert_dict(sites, "c.com", strlen("c.com"));
    frk_dict_t* www_c = frk_insert_dict(c, "www.c.com", strlen("www.c.com"));
    frk_insert_int(www_c, "timeout", strlen("timeout"), 200);
    frk_insert_int(www_c, "buffer", strlen("buffer"), 10240);

    char *buffer = calloc(1024, 100);

    // for each 
    frk_iter_t tmp, *i = frk_iterator(black, NULL, &tmp);
    for (; i != NULL; i = frk_iterator(black, i, &tmp)) {
        // i->node->key->data may not ends with '\0' don't do that in product
        printf("%s: %ld\n", i->node->key->data, i->node->i);
    }

    frk_dict_dump(sites, buffer, 102400);
    printf("%s\n", buffer);

    // todo foreach
}

int main() {
    slab_test();
    dict_test();
    app_test();

    return 0;
}
