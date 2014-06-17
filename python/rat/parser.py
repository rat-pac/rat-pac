'''Code for parsing ROOT-style selector strings, as are used in TTree::Draw.'''
import operator
import itertools

def unpack_attributes(identifier):
    '''Converts an identifier string into a list of attribute parts.

    >>> unpack_attributes('mc.numPE')
    ['mc', 'numPE']
    >>> unpack_attributes('mc.particle.pos.X()')
    ['mc', 'particle', 'pos', 'X()']
    '''
    return identifier.split('.')

def is_loopable(obj):
    '''Returns true if this an object we should loop over by default
    when evaluating attribute lookups.  This includes lists and
    tuples, but not strings or TVector3

    >>> is_loopable('a')
    False
    >>> is_loopable([1,2,3])
    True
    >>> is_loopable(1)
    False
    >>> is_loopable((1,2,3))
    True
    '''
    try:
        iter(obj)
        iterable = True
    except TypeError:
        iterable = False

    if isinstance(obj, (str, unicode)):
        return False
    elif obj.__class__.__name__.endswith('TVector3'):
        return False
    elif iterable:
        return True
    else:
        return False

def merge_holes(list_of_lists):
    '''Combine a list of lists such that the final list has the same
    length as the longest list, and it contains all the non-null entries
    of the individiual lists.

    >>> merge_holes([[None, 1, 2], [0]])
    [0, 1, 2]
    '''
    max_len = max(map(len, list_of_lists))
    merged = [None] * max_len

    # Merge all these lists into one list.  Require that
    # for each slot there is either zero or one entry.
    for i in xrange(max_len):
        # Filter out non-null 
        non_null = [ entry[i] for entry in list_of_lists 
                     if i < len(entry) and entry[i] is not None ]
        if len(non_null) == 0:
            merged[i] = None
        elif len(non_null) == 1:
            merged[i] = non_null[0]
        else:
            assert False, 'Two attributes have slot #%d' % i
    return merged

def sum_non_null(a, b):
    '''If ``a`` and ``b`` are not ``None``, return ``a + b``.  Otherwise return
    ``None``.'''
    if a is None or b is None:
        return None
    else:
        return a + b

def create_evaluation_tree(*selectors):
    '''Returns an AttributeNode tree that evaluates the list of selector strings
    and returns the values in the same order.

    >>> str(create_evaluation_tree('mc.numPE', 'ev.qPE', 'ev.cut'))
    '(mc : (numPE : 0), ev : (qPE : 1, cut : 2))'
    '''
    root = AttributeNode('')

    all_parts = [ unpack_attributes(selector) for selector in selectors ]
    
    for slot, selector_parts in enumerate(all_parts):
        node = root
        optional = False
        for part in selector_parts:
            if part.endswith('?'):
                optional = True

            # All parts after the first one with a ? need to have a ?
            if optional and not part.endswith('?'):
                part += '?'

            child = node.find_child(part)
            if child is None:
                child = AttributeNode(part)
                node.child.append(child)
            node = child
        # node is the leaf
        node.slot = slot

    # remove unnecessary empty top node
    if len(root.child) == 0:
        return root.child[0]
    else:
        return root

class AttributeNode(object):
    '''Represents a prefix-tree of attribute lookups.'''

    def __init__(self, attribute, slot=None, child=None):
        '''
          ``attribute``: str
            Name of attribute
            
          ``slot``: integer or None
            If this is a leaf node, it needs a slot number
            that indicates its index in the top-level array of
            attribute values.  If this node has children,
            ``slot`` should be set to None.
          ``child``: list of AttributeNode objects
            The list is copied.  Defaults to empty list.
        '''
        self.attribute = attribute
        self.slot = slot
        if child is None:
            self.child = []
        else:
            self.child = list(child)
        assert not (slot is not None and len(self.child) > 0)
    
    def flatten(self):
        '''Returns a list of strings of full attribute lookup strings
        sorted by their slot ID number.  ``None`` will fill empty
        slots.

        Child attribute lookups are joined to their parents with periods.

        >>> AttributeNode('a', slot=2).flatten()
        [None, None, 'a']
        >>> AttributeNode('a', child=[AttributeNode('b',0),AttributeNode('c',3)]).flatten()
        ['a.b', None, None, 'a.c']
        '''
        if self.slot is not None:
            full_attributes = [None] * (self.slot + 1)
            full_attributes[self.slot] = self.attribute
            return full_attributes
        elif len(self.child) == 0:
            return []
        else:
            child_attributes = [ child.flatten() for child in self.child ]
            max_len = max(map(len, child_attributes))
            full_attributes = [None] * max_len
            if self.attribute == '':
                prefix = ''
            else:
                prefix = self.attribute + '.'

            return [ sum_non_null(prefix, element)
                     for element in 
                     merge_holes(child_attributes) ]

    def get(self, obj):
        '''Returns an iterator over the immediate values for the attribute
        represented by this node.  If getattr(obj,self.attribute) is a list
        or tuple, the returned iterator will return each element separately.
        
        >>> class Struct(object): pass
        >>> a = Struct()
        >>> a.foo = 1
        >>> a.bar = [2,3]
        >>> list(AttributeNode('foo').get(a))
        [1]
        >>> list(AttributeNode('bar').get(a))
        [2, 3]
        >>> list(AttributeNode('@bar').get(a))
        [[2, 3]]
        >>> list(AttributeNode('split()').get('a b c'))
        [['a', 'b', 'c']]
        '''
        attribute = self.attribute
        iterate_list = True
        call_function = False
        none_is_one_element = False
        
        # Identify special kinds of attributes
        if attribute.startswith('@'):
            iterate_list = False
            attribute = attribute[1:]
        if attribute.endswith('?'):
            none_is_one_element = True
            attribute = attribute[:-1]
        if attribute.endswith('()'):
            call_function = True
            attribute = attribute[:-2]

        if self.attribute == '':
            yield obj # Pass through empty attribute
        elif obj is not None:
            value = getattr(obj, attribute)
            # PyROOT does not return a None when it gives you a null ptr anymore
            # Have to convert to a boolean to test!
            if not bool(value) and 'RAT' in value.__class__.__name__:
                value = None
                
            if is_loopable(value) and iterate_list:
                if len(value) == 0 and none_is_one_element:
                    yield None
                else:
                    for element in value:
                        yield element
            else:
                if call_function:
                    yield value()
                else:
                    yield value
        elif none_is_one_element:
            yield None
        else:
            pass # equivalent to zero length list

    def eval(self, obj):
        '''Returns a list of lists of attribute contents for this tree, given
        the slot numbers of the leaf nodes.

        Lists are implicitly looped over, which is why this function returns
        a list of attribute evaluation lists.  When the children of a node
        return different numbers of entries, the parent will return
        the Cartesian product of these values.

        >>> class Struct(object): pass
        >>> obj = Struct()
        >>> obj.a = Struct()
        >>> obj.a.b = 4
        >>> obj.a.c = 'test'
        >>> tree = AttributeNode('a', child=[AttributeNode('b',0),AttributeNode('c',1)])
        >>> tree.eval(obj)
        [[4, 'test']]
        >>> obj.a.b = [4,5]
        >>> obj.a.c = ['a','b','c']
        >>> tree.eval(obj)
        [[4, 'a'], [4, 'b'], [4, 'c'], [5, 'a'], [5, 'b'], [5, 'c']]
        '''
        content_list = []
        if self.slot is not None:
            for v in self.get(obj):
                values = [None] * (self.slot + 1)
                values[self.slot] = v
                content_list.append(values)
        elif len(self.child) == 0:
            pass
        else:
            for v in self.get(obj):
                child_content_lists = [ child.eval(v) for child in self.child ]

                nentries_each = map(len, child_content_lists)
                nentries_total = reduce(operator.mul, nentries_each)
                if nentries_total > 1000000:
                    raise Exception("parser: Adding %d rows for one event to "
                                    "ntuple \"%s\"! Aborting to prevent memory "
                                    "explosion." 
                                    % (nentries_total, str(self)))

                for value_lists in itertools.product(*child_content_lists):
                    content_list.append(merge_holes(value_lists))

        return content_list

    def find_child(self, child_attr):
        '''Returns AttributeNode for child with attribute ``child_attr`` or
        ``None`` if it does not exist.

        >>> root = AttributeNode('a', child=[AttributeNode('b',0),AttributeNode('c',1)])
        >>> str(root.find_child('b'))
        'b : 0'
        >>> str(root.find_child('d'))
        'None'
        '''
        for child in self.child:
            if child.attribute == child_attr:
                return child
        return None

    def __str__(self):
        '''Returns strings representation of this object which recursively
        stringifies all children.

        Examples:
        >>> str(AttributeNode('a'))
        'a : ()'
        >>> str(AttributeNode('a', slot=0))
        'a : 0'
        >>> str(AttributeNode('a', child=[AttributeNode('b',0),AttributeNode('c',1)]))
        'a : (b : 0, c : 1)'
        >>> str(AttributeNode('', child=[AttributeNode('b',0),AttributeNode('c',1)]))
        '(b : 0, c : 1)'
        '''
        if self.slot is None:
            child_str = ', '.join(map(str, self.child))
            if self.attribute == '':
                return '(%s)' % child_str
            return '%s : (%s)' % (self.attribute, child_str)
        else:
            return '%s : %d' % (self.attribute, self.slot)
