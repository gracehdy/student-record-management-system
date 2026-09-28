# Student Record Management System

Aplikasi berbasis *command line* untuk mengelola data mahasiswa **BEE University**, dibangun menggunakan struktur data **B-Tree** yang diimplementasikan sendiri dari nol dalam bahasa C/C++. Proyek ini menunjukkan bagaimana B-Tree dapat dipakai untuk menyimpan, mencari, dan menghapus data secara efisien dengan struktur pohon yang selalu seimbang.

---

## Fitur

Program menyediakan menu interaktif berikut:

| No | Menu | Deskripsi |
|----|------|-----------|
| 1 | **Insert Student** | Menambahkan data mahasiswa baru dengan validasi input. |
| 2 | **View Records** | Menampilkan seluruh data mahasiswa secara terurut berdasarkan ID (*in-order traversal*). |
| 3 | **Search by ID** | Mencari data mahasiswa berdasarkan ID. |
| 4 | **Delete by ID** | Menghapus data mahasiswa berdasarkan ID. |
| 5 | **Exit** | Keluar dari program. |

### Data yang Disimpan

| Field | Keterangan |
|-------|------------|
| `ID` | ID mahasiswa, unik, maksimal 10 karakter |
| `Name` | Nama mahasiswa, 5–50 karakter |
| `Major` | Jurusan, 5–50 karakter |
| `GPA` | IPK, rentang 0.00–4.00 |

### Validasi Input

- Nama dan jurusan harus 5–50 karakter.
- GPA harus berada di antara 0.00 dan 4.00.
- ID tidak boleh lebih dari 10 karakter dan tidak boleh duplikat.

## Implementasi B-Tree

- **Minimum degree (t)** = `3` (dapat diubah lewat `MIN_DEGREE`).
- Setiap node menyimpan maksimal `2t - 1` key dan minimal `t - 1` key (kecuali root).
- Key yang digunakan adalah **ID mahasiswa** (string), dibandingkan dengan `strcmp`.
- Setiap key berpasangan dengan satu `StudentRecord`.

Operasi yang diimplementasikan:

| Operasi | Fungsi |
|---------|--------|
| Insert | `insert_student`, `insertNonFull`, `splitChild` |
| Search | `search_student`, `search_node` |
| Delete | `deleteKey`, `deleteFromNode`, `fillChild`, `borrowFromPrev`, `borrowFromNext`, `mergeChildren` |
| Traversal | `traverse` (menampilkan data terurut) |

## Struktur Repositori

```
student-record-management-system/
├── AOL_DS.cpp     # Source code utama
└── laporan/       # Laporan proyek
```

## Cara Menjalankan

### Prasyarat

Compiler C/C++ seperti `gcc` atau `g++`.

### Kompilasi

```bash
g++ AOL_DS.cpp -o student_record
```

### Menjalankan

```bash
./student_record          # Linux / macOS
student_record.exe        # Windows
```

## Teknologi

- Bahasa C/C++
- Struktur data B-Tree (implementasi manual dengan alokasi memori dinamis)
