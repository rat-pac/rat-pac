from rat import ROOT, RAT

def make_typed_array(l):
    '''Takes a list and makes it into the appropriate array
    using STL vector types.  ``l`` must have length > 0!'''
    if len(l) == 0:
        raise TypeError('zero length array has no type')
    first_element = l[0]
    if isinstance(first_element, str):
        array = ROOT.std.vector('string')()
        array_type = str
    elif isinstance(first_element, int):
        array = ROOT.std.vector('int')()
        array_type = int
    elif isinstance(first_element, float):
        array = ROOT.std.vector('double')()
        array_type = float
    else:
        raise TypeError('unknown array element type')

    for v in l:
        array.push_back(v)
    return array_type, array

def load_table(table, db=None):
    '''Load a RATDB table in the form of a Python dictionary.
    If ``db`` is not specified, the global default database is used.'''

    if db is None:
        db = RAT.DB.Get()

    name = table['name']
    index = table.get('index', '')
    dbtable = RAT.DBTable(name, index)
    dbtable.SetRunRange(*table['run_range'])

    for key, value in table.items():
        if key in ['name', 'index', 'run_range']:
            continue

        if isinstance(value, str):
            dbtable.SetS(key, value)
        elif isinstance(value, int):
            dbtable.SetI(key, value)
        elif isinstance(value, float):
            dbtable.SetD(key, value)
        elif isinstance(value, (list, tuple)):
            array_type, array = make_typed_array(value)
            if array_type == str:
                dbtable.SetSArray(key, array)
            elif array_type == int:
                dbtable.SetIArray(key, array)
            elif array_type == float:
                dbtable.SetDArray(key, array)
            else:
                raise TypeError('unknown array type in "%s"' % key)
        else:
            raise TypeError('unknown type in "%s"' % key)

    ROOT.SetOwnership(dbtable, False)
    db.LoadTable(dbtable)
