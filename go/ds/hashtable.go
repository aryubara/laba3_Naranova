package ds

import (
	"errors"
	"strings"
)

// ErrKeyNotFound возвращается, если ключ отсутствует в хеш-таблице.
var ErrKeyNotFound = errors.New("ds: key not found")

// Djb2 - хеш-функция DJB2 (h = h*33 + c).
func Djb2(key string) uint64 {
	h := uint64(5381)
	for i := 0; i < len(key); i++ {
		h = h*33 + uint64(key[i])
	}
	return h
}

type hashNode struct {
	key, value string
	next       *hashNode
}

// HashTable - ассоциативный массив (ключ -> значение) на цепочках; расширяется при заполнении > 0.75.
type HashTable struct {
	buckets []*hashNode
	size    int
}

// NewHashTable создаёт таблицу с заданным числом корзин (минимум 1).
func NewHashTable(buckets int) *HashTable {
	if buckets < 1 {
		buckets = 1
	}
	return &HashTable{buckets: make([]*hashNode, buckets)}
}

func (h *HashTable) TypeName() string { return "HashTable" }
func (h *HashTable) Size() int        { return h.size }
func (h *HashTable) BucketCount() int { return len(h.buckets) }

func (h *HashTable) idx(key string, n int) int { return int(Djb2(key) % uint64(n)) }

func (h *HashTable) Contains(key string) bool {
	for n := h.buckets[h.idx(key, len(h.buckets))]; n != nil; n = n.next {
		if n.key == key {
			return true
		}
	}
	return false
}

// Get возвращает значение по ключу или ErrKeyNotFound.
func (h *HashTable) Get(key string) (string, error) {
	for n := h.buckets[h.idx(key, len(h.buckets))]; n != nil; n = n.next {
		if n.key == key {
			return n.value, nil
		}
	}
	return "", ErrKeyNotFound
}

// Insert вставляет пару (key, value). Если ключ уже есть — обновляет значение и возвращает false.
func (h *HashTable) Insert(key, value string) bool {
	i := h.idx(key, len(h.buckets))
	for n := h.buckets[i]; n != nil; n = n.next {
		if n.key == key {
			n.value = value
			return false
		}
	}
	if (h.size+1)*4 > len(h.buckets)*3 {
		h.rehash(len(h.buckets) * 2)
	}
	i = h.idx(key, len(h.buckets))
	h.buckets[i] = &hashNode{key: key, value: value, next: h.buckets[i]}
	h.size++
	return true
}

func (h *HashTable) Erase(key string) bool {
	i := h.idx(key, len(h.buckets))
	var prev *hashNode
	for n := h.buckets[i]; n != nil; prev, n = n, n.next {
		if n.key == key {
			if prev == nil {
				h.buckets[i] = n.next
			} else {
				prev.next = n.next
			}
			h.size--
			return true
		}
	}
	return false
}

func (h *HashTable) rehash(n int) {
	fresh := make([]*hashNode, n)
	for _, head := range h.buckets {
		for cur := head; cur != nil; {
			next := cur.next
			i := h.idx(cur.key, n)
			cur.next = fresh[i]
			fresh[i] = cur
			cur = next
		}
	}
	h.buckets = fresh
}

// MaxChainLength - длина самой длинной цепочки.
func (h *HashTable) MaxChainLength() int {
	m := 0
	for _, head := range h.buckets {
		l := 0
		for n := head; n != nil; n = n.next {
			l++
		}
		if l > m {
			m = l
		}
	}
	return m
}

func (h *HashTable) ToSlice() []string {
	out := make([]string, 0, h.size)
	for _, head := range h.buckets {
		for n := head; n != nil; n = n.next {
			out = append(out, n.key+"="+n.value)
		}
	}
	return out
}

func (h *HashTable) Clear() {
	for i := range h.buckets {
		h.buckets[i] = nil
	}
	h.size = 0
}

func (h *HashTable) Append(entry string) error {
	k, v, ok := strings.Cut(entry, "=")
	if !ok {
		h.Insert(k, "")
	} else {
		h.Insert(k, v)
	}
	return nil
}
