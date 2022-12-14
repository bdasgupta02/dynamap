# Dynamap: A dynamically resized and thread-safe hashmap
This hashmap implementation dynamically resizes its capacity on a fine-grained level, and provides thread-safety to all operations.

## Usage
### Default example
```cpp
// <key, value>
dyna::map<int, int> hashmap;

hashmap.set(2, 502);

// returns 502
int value = hashmap[2];
```

### Custom hash function
```cpp
// hash function type, with default set to std::hash
dyna::map<ObjectType, int, Hasher> hashmap;

hashmap.set(obj, 502);

int value = hashmap[obj];
```

### Thread safety
```cpp
// either thread::safe or thread::unsafe, with default set to safe
dyna::map<int, int, std::hash, dyna::thread::unsafe> hashmap; 

hashmap.set(2, 502);

int value = hashmap[2];
```

## Underlying structure
![Underlying data structure](https://drive.google.com/uc?export=download&id=1G9dm5lTyADPL_TsKM0WTz4J0XNE0LTWv)
Data buckets have a linked list implementation due to the fact that sub-tables will be automtically resized to result in a more sparse distribution.

## Member functions
#### Constructors
<table>
  <tbody>
    <tr>
      <td><code>map()</code></td>
      <td>Creates a map</td>
    </tr>
    <tr>
      <td><code>map(custom_capacity)</code></td>
      <td>Creates a map with specific top-level size</td>
    </tr>
  </tbody>
</table>

#### Iterators
<table>
  <tbody>
    <tr>
      <td><code>begin()</code></td>
      <td>Returns iterator to the first element</td>
    </tr>
    <tr>
      <td><code>end()</code></td>
      <td>Returns iterator to the end</td>
    </tr>
  </tbody>
</table>

#### Getter and setter
<table>
  <tbody>
    <tr>
      <td><code>operator[key]</code></td>
      <td>Returns value at key</td>
    </tr>
    <tr>
      <td><code>set(key, value)</code></td>
      <td>Sets key with value</td>
    </tr>
    <tr>
      <td><code>exists(key)</code></td>
      <td>Returns if a key exists in the map or not</td>
    </tr>
  </tbody>
</table>

#### Erasing
<table>
  <tbody>
    <tr>
      <td><code>erase(key)</code></td>
      <td>Erases pair with specified key</td>
    </tr>
  </tbody>
</table>

#### Capacity
<table>
  <tbody>
    <tr>
      <td><code>size()</code></td>
      <td>Returns size of added elements</td>
    </tr>
    <tr>
      <td><code>empty()</code></td>
      <td>Returns if the map is empty or not</td>
    </tr>
    <tr>
      <td><code>max_size()</code></td>
      <td>Returns max capacity of top-level map</td>
    </tr>
    <tr>
      <td><code>max_size_deep()</code></td>
      <td>Returns <span style="font-weight:bold">current</span> max capacity of all combined sub-tables</td>
    </tr>
  </tbody>
</table>

## Performance
Benchmarked with a high resolution clock 
Specs: Apple MacBook Pro M1 Pro (ARM)
```
Type          Time(ns)
       
```
- In progress

## Iterator usage
This implementation includes a FIFO-based ordering using iterators. This also works with ranged loops. Example usage:
```cpp
map.set(0, 0);
map.set(1, 1);

auto it = map.begin();
++it;

int key = it->key(); // 1
int value = it->value(); // 1
```

## Assumptions
- Value type has zero argument constructor

## Incoming additions
- Support for ranged loops through better iterators **[DONE]**
- Compiler optimizations
- More functionality (including `clear()`, optimization settings)
- Reduction of overhead further
