import os.path

kPending = 0
kRunning = 1
kSuccess = 2
kFail    = 3

class TaskList:
    def __init__(self):
        self.index = {}
        self.pending = []
        self.running = []
        self.success = []
        self.fail = []

    def get_by_id(self, id):
        return self.index[id]

    def update_status(self):
        for t in running:
            if t.status == kSuccess:
                self.running.remove(t)
                self.success.append(t)
            elif t.status == kFail:
                self.running.remove(t)
                self.fail.append(t)
            elif t.status == kPending:
                print 'Warning: Task', name, \
                      'has reverted from running to pending!'
                self.running.remove(t)
                self.fail.append(t)

class Task:
    
    def __init__(self, name, desc, cmd, success_file, dependencies=[]):
        self.name = name
        self.id = self.next_task_id()
        self.cmd = cmd
        self.jobid = -1
        self.success_file = success_file
        self.__status = kPending
        self.dependencies = dependencies

    last_task_id = -1
    def next_task_id(cls):
        cls.last_task_id += 1
        return cls.last_task_id
    next_task_id = classmethod(next_task_id)

    def get_status(self):
        if self.__status == kRunning:
            self.update_status()
        return self.__status
    def set_status(self, status): self.__status = status
    status = property(get_status, set_status, doc='Current status of task')

    def update_status(self):
        if self.__status != kRunning:
            return
        elif self.jobid < 0:  # Not a grid job, so is run synchronously
            if os.path.isfile(self.success_file): self.status = kSuccess
            else: self.status = kFail

    def __repr__(self):
        return 'Task %d: %s' % (self.id, self.cmd)

def file_subst(fname_in, fname_out, subst):
    fin = file(fname_in)
    contents = fin.read()
    fin.close()
    
    for key in subst.keys():
        contents = contents.replace("%"+key+"%", subst[key])

    fout = file(fname_out, "w")
    fout.write(contents)
    fout.close()


def create_data_tasks(desc_list, local_config):
    events_per_job = local_config['events_per_job']
    newtasks = { }
    
    for shortname, datadesc in desc_list.items():
        macro = os.path.join(datadesc['dir'], datadesc['macro'])
        workdir = datadesc['dir']
        params = {}

        # Break into several jobs
        numjobs = datadesc['events'] / events_per_job
        for i in range(numjobs):
            params['jobnum'] = '%03d' % i
            jobfile = 'job%03d.mac' % i
            logfile = 'job%03d.log' % i

            # Create macro file
            full_jobfile = os.path.join(workdir, jobfile)
            file_subst(macro, full_jobfile, params)

            # Create associated task object
            t = Task(name = shortname,
                     desc = datadesc['desc'],
                     cmd  = './run_data.sh %s %03d' % (workdir, i),
                     success_file = os.path.join(workdir, 'success.%03d' % i))
                                                        
            # Add to list
            newtasks[t.id] = t

    return newtasks
