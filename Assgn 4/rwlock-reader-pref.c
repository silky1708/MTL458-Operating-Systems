#include "rwlock.h"

void InitalizeReadWriteLock(struct read_write_lock * rw)
{
  //	Write the code for initializing your read-write lock.
	rw->readers = 0;
	sem_init(&rw->readlock, 0, 1);
	sem_init(&rw->writelock, 0, 1);
	sem_init(&rw->commonlock, 0, 1);
	sem_init(&rw->upd_lock, 0, 1);
	rw->writers = 0;
}

void ReaderLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the reader.
	sem_wait(&rw->readlock);
	rw->readers++;
	if(rw->readers == 1)
		sem_wait(&rw->writelock);
	sem_post(&rw->readlock);
}

void ReaderUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the reader.
	sem_wait(&rw->readlock);
	rw->readers--;
	if(rw->readers == 0)
		sem_post(&rw->writelock);
	sem_post(&rw->readlock);
}

void WriterLock(struct read_write_lock * rw)
{
  //	Write the code for aquiring read-write lock by the writer.
	sem_wait(&rw->writelock);
}

void WriterUnlock(struct read_write_lock * rw)
{
  //	Write the code for releasing read-write lock by the writer.
	sem_post(&rw->writelock);
}
